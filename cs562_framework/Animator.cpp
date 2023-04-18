#include "stdafx.h"

#include "Animation.h"
#include "Bone.h"
#include "AnimationData.h"

#include "Animator.h"

#include <glm/gtx/matrix_decompose.hpp>

Animator::Animator() :
	curr_anim{ nullptr },
	curr_time{ 0.0f },
	set_counter{ 0 } {
	// MAX_BONES = 100 from vertex shader
	bone_positions.reserve(100);
	final_bone_matrices.reserve(100);
	for (uint32_t i = 0; i < 100; ++i) {
		final_bone_matrices.push_back(glm::mat4{ 1.0f });
	}
	constraints.reserve(5);
	angle_rotated.reserve(5);
	for (uint32_t i = 0; i < 5; ++i) {
		constraints.push_back(glm::vec2{});
		angle_rotated.push_back(0.0f);
	}
}

void Animator::UpdateAnimation(float dt) {
	if (curr_anim) {
		curr_time += curr_anim->GetTicksPerSecond() * dt;
		curr_time = fmod(curr_time, curr_anim->GetDuration());
		bone_positions.clear();
		set_counter = 0;
		CalculateBoneTransform(curr_anim->GetRootNode(), glm::mat4{ 1.0f }, false);
	}
}

void Animator::PlayAnimation(std::shared_ptr<Animation> p_anim) {
	curr_anim = p_anim;
	curr_time = 0.0f;
}

void Animator::CalculateBoneTransform(std::shared_ptr<AssimpNodeData> node, glm::mat4 parent_transform, bool ik_update) {
	std::string node_name = node->name;
	glm::mat4 node_transform = node->transform;

	std::shared_ptr<Bone> bone = curr_anim->FindBone(node_name);
	if (bone) {
		// get local transform of each bone
		if (not ik_update) {
			bone->Update();
		}
		node_transform = bone->GetLocalTransform();
	}

	// transform wrt world
	glm::mat4 global_transform = parent_transform * node_transform;

	std::unordered_map<std::string, BoneInfo> const& bone_info_map = curr_anim->GetBoneInfoMap();
	if (bone_info_map.find(node_name) != bone_info_map.end()) {
		int index = bone_info_map.at(node_name).id;
		glm::mat4 offset = bone_info_map.at(node_name).offset;
		final_bone_matrices[index] = global_transform * offset;

		// bone positions for drawing skeleton
		bone_positions.push_back(std::make_tuple(set_counter, global_transform * glm::vec4{ 0.0f, 0.0f, 0.0f, 1.0f }));
	}

	// update transforms of all children recursively
	for (unsigned i = 0; i < node->children_count; ++i) {
		CalculateBoneTransform(node->children[i], global_transform, ik_update);
	}

	// increment "set" of bones
	++set_counter;
}

// get list of manipulators from animation
void Animator::SetupIK(std::string const& end_effector) {
	manipulators = curr_anim->GetManipulators(end_effector);

	constraints[0] = glm::vec2{ -10.0f, 10.0f };
	constraints[1] = glm::vec2{ -30.0f, 30.0f };
	constraints[2] = glm::vec2{ 0.0f, 110.0f };
	constraints[3] = glm::vec2{ -90.0f, 60.0f };
	constraints[4] = glm::vec2{ -1.0f, 1.0f };
}

void Animator::ResetIK() {
	for (uint32_t i = 0; i < 5; ++i) {
		angle_rotated[i] = 0.0f;
	}
}

void Animator::SolveIK(glm::vec3 const& target_pos, float dt) {
	bone_positions.clear();
	set_counter = 0;

	std::unordered_map<std::string, BoneInfo> const& bone_info_map = curr_anim->GetBoneInfoMap();

	for (unsigned int i = 0; i < manipulators.size() - 1; ++i) {
		// getting transformation matrix in end effector's local space
		glm::mat4 transform_in_ee_local_space{ 1.0f };
		for (int j = i; j >= 0; --j) {
			std::shared_ptr<Bone> bone = curr_anim->FindBone(manipulators[j]->name);
			transform_in_ee_local_space *= bone->GetLocalTransform();
		}

		// vector from current joint to current position of end effector
		glm::vec3 v_ck { 
			transform_in_ee_local_space[3][0],
			transform_in_ee_local_space[3][1],
			transform_in_ee_local_space[3][2],
		};

		std::shared_ptr<Bone> parent_bone = curr_anim->FindBone(manipulators[i + 1]->name);
		int parent_id = bone_info_map.at(manipulators[i + 1]->name).id;
		glm::mat4 parent_transform = final_bone_matrices[parent_id] *
										glm::inverse(bone_info_map.at(parent_bone->GetBoneName()).offset);
		// vector from current joint to final position of end effector
		glm::vec3 v_dk = glm::inverse(parent_transform) * glm::vec4{ target_pos, 1.0f };

		v_ck = glm::normalize(v_ck);
		v_dk = glm::normalize(v_dk);

		// axis of rotation = cross(v_ck, v_dk)
		glm::vec3 axis = glm::cross(v_ck, v_dk);
		// angle to be rotated = dot(v_ck, v_dk)
		float angle = acosf(glm::dot(v_ck, v_dk)) * dt;


		// clamp angle within constraints
		if (angle_rotated[i] + glm::degrees(angle) < constraints[i].x ||
			angle_rotated[i] + glm::degrees(angle) > constraints[i].y) {
			angle = 0.0f;
		}
		else {
			// keep track of how much this manipulator has been rotated
			angle_rotated[i] += glm::degrees(angle);
		}

		// exit condition
		if (glm::length(axis) < 0.01f) {
			break;
		}
		axis = glm::normalize(axis);

		glm::mat4 rotation_mat = glm::mat4{ glm::angleAxis(angle, axis) };
		rotation_mat *= transform_in_ee_local_space;

		// transform rotation matrix back to local space of manipulator
		for (unsigned int j = 0; j < i; ++j) {
			std::shared_ptr<Bone> bone = curr_anim->FindBone(manipulators[j]->name);
			rotation_mat *= glm::inverse(bone->GetLocalTransform());
		}
		curr_anim->FindBone(manipulators[i]->name)->SetLocalTransform(rotation_mat);

		// update final bone matrix
		for (int j = i; j >= 0; --j) {
			int curr_index = bone_info_map.at(manipulators[j]->name).id;
			int parent_index = bone_info_map.at(manipulators[j + 1]->name).id;

			std::shared_ptr<Bone> curr_bone = curr_anim->FindBone(manipulators[j]->name);
			std::shared_ptr<Bone> parent_bone = curr_anim->FindBone(manipulators[j + 1]->name);
			glm::mat4 global_transform = final_bone_matrices[parent_index]
											* glm::inverse(bone_info_map.at(parent_bone->GetBoneName()).offset);
			global_transform *= curr_bone->GetLocalTransform();
			final_bone_matrices[curr_index] = global_transform * bone_info_map.at(curr_bone->GetBoneName()).offset;
		}
	}

	CalculateBoneTransform(curr_anim->GetRootNode(), glm::mat4{ 1.0f }, true);
}

std::vector<glm::mat4> const& Animator::GetFinalBoneMatrices() const {
	return final_bone_matrices;
}

std::vector<std::tuple<uint32_t, glm::vec3>> const& Animator::GetBonePositions() const {
	return bone_positions;
}