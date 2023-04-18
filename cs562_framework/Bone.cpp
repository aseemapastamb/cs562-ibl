#include "stdafx.h"

#include "AssimpHelper.h"

#include "Bone.h"

Bone::Bone(std::string const& name, int id, aiNodeAnim* channel) :
	name{ name },
	id{ id },
	local_transform{ 1.0f },
	num_key_frames{ channel->mNumPositionKeys },
	num_incr{ 1 },
	itr{ 0 },
	keyframe_index{ 0 },
	curr_vqs{},
	prev_vqs{} {
	// initializing keyframe data
	vqs_key_frames.reserve(num_key_frames);
	for (uint32_t i = 0; i < num_key_frames; ++i) {
		aiVector3D ai_pos = channel->mPositionKeys[i].mValue;
		aiQuaternion ai_rot = channel->mRotationKeys[i].mValue;
		aiVector3D ai_scale = channel->mScalingKeys[i].mValue;
		VQS vqs;
		vqs.v = AssimpHelper::GetGLMVec3(ai_pos);
		vqs.q = AssimpHelper::GetQuat(ai_rot);
		vqs.s = ai_scale.x;
		vqs_key_frames.push_back(vqs);
	}

	// incremental VQS preprocessing
	VQS_c.reserve(num_key_frames);
	for (uint32_t i = 0; i < num_key_frames - 1; ++i) {
		// for each pair of keyframes
		VQS vqs_c;

		// position v_c = (v_n - v_0) / n
		aiVector3D v_c = (channel->mPositionKeys[i + 1].mValue - channel->mPositionKeys[i].mValue) / static_cast<float>(num_incr);
		vqs_c.v = AssimpHelper::GetGLMVec3(v_c);

		// rotation q_c = [cos(beta), sin(beta)v] (iSlerp)
		// q_0 dot q_n = cos(alpha), beta = alpha / n
		// v = ( (s_0 v_n) - (s_n v_0) + (v_0 cross v_n) ) / sin(alpha)
		Quaternion q_0 = AssimpHelper::GetQuat(channel->mRotationKeys[i].mValue);
		q_0.Normalize();
		Quaternion q_n = AssimpHelper::GetQuat(channel->mRotationKeys[i + 1].mValue);
		q_n.Normalize();
		float q_0_Dot_q_n = q_0.Dot(q_n);
		q_0_Dot_q_n = glm::clamp(q_0_Dot_q_n , -0.99f, 0.99f);
		float alpha = acosf(q_0_Dot_q_n);
		float beta = alpha / static_cast<float>(num_incr);
		glm::vec3 v = sinf(beta) * ((q_0.s * q_n.v) - (q_n.s * q_0.v) + (glm::cross(q_0.v, q_n.v))) / sinf(alpha);
		vqs_c.q = Quaternion{ cosf(beta), v };

		// scale s_c = (s_n / s_0) ^ (1 / n)
		vqs_c.s = powf((channel->mScalingKeys[i + 1].mValue.x / channel->mScalingKeys[i].mValue.x), (1.0f / static_cast<float>(num_incr)));

		VQS_c.push_back(vqs_c);
	}
}

void Bone::Update() {
	// first iteration between keyframes
	if (itr == 0) {
		curr_vqs = vqs_key_frames[keyframe_index];
	}
	// iterations within keyframe finished
	if (itr > num_incr) {
		itr = 0;
		keyframe_index = (keyframe_index + 1) % (num_key_frames - 1);
		curr_vqs = vqs_key_frames[keyframe_index];
	}
	else {
		// v_curr = v_c + v_prev
		curr_vqs.v = VQS_c[keyframe_index].v + prev_vqs.v;
		// q_curr = q_c * q_prev
		curr_vqs.q = VQS_c[keyframe_index].q.Multiply(prev_vqs.q);
		// s_curr = s_c * s_prev
		curr_vqs.s = VQS_c[keyframe_index].s * prev_vqs.s;
	}
	prev_vqs = curr_vqs;
	++itr;

	// updating transform matrix
	glm::mat4 translation_mat = glm::translate(glm::mat4{ 1.0f }, curr_vqs.v);
	Quaternion rot = curr_vqs.q;
	rot.Normalize();
	glm::mat4 rotation_mat = glm::mat4{ glm::transpose(rot.ToMat3()) };
	glm::mat4 scale_mat = glm::scale(glm::mat4{ 1.0f }, glm::vec3{ curr_vqs.s });

	local_transform = translation_mat * rotation_mat * scale_mat;
}

glm::mat4 Bone::GetLocalTransform() const {
	return local_transform;
}

void Bone::SetLocalTransform(glm::mat4 new_local_transform) {
	local_transform = new_local_transform;
}

std::string const& Bone::GetBoneName() const {
	return name;
}

int Bone::GetBoneID() const {
	return id;
}

void Bone::SetNumIncr(int _num_incr) {
	num_incr = _num_incr;
}
