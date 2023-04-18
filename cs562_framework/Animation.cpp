#include "stdafx.h"

#include "AnimationData.h"
#include "VQS.h"
#include "Bone.h"
#include "Mesh.h"
#include "Model.h"

#include "Animation.h"

#include "AssimpHelper.h"
#include <queue>

Animation::Animation() :
	duration{ 0.0f },
	ticks_per_sec{ 0.0f },
	root_node{} {
}

void Animation::LoadAnimation(std::string const& anim_path, Model* model) {
	Assimp::Importer importer;
	aiScene const* scene = importer.ReadFile(anim_path, aiProcess_Triangulate |
		aiProcess_GenSmoothNormals |
		aiProcess_FlipUVs |
		aiProcess_CalcTangentSpace);
	assert(scene && scene->mRootNode);
	aiAnimation* p_anim = scene->mAnimations[0];
	duration = static_cast<float>(p_anim->mDuration);
	ticks_per_sec = static_cast<float>(p_anim->mTicksPerSecond);
	m_anim_path = anim_path;
	aiMatrix4x4 global_transform = scene->mRootNode->mTransformation;
	global_transform = global_transform.Inverse();
	root_node = ReadHierarchyData(nullptr, scene->mRootNode);
	ReadMissingBones(p_anim, *model);
}

std::shared_ptr<Bone> Animation::FindBone(std::string const& name) {
	for (auto bone : bones) {
		if (bone->GetBoneName() == name) {
			return bone;
		}
	}
	return nullptr;
}

std::vector<std::shared_ptr<AssimpNodeData>> Animation::GetManipulators(std::string const& end_effector_name) {
	std::shared_ptr<AssimpNodeData> manipulator = FindNodeData(end_effector_name);

	// priority list - only manipulate bones from shoulder till index finger
	std::vector<std::shared_ptr<AssimpNodeData>> manipulators;
	while (manipulator->name != "mixamorig1_Spine2") {
		manipulators.push_back(manipulator);
		manipulator = manipulator->parent;
	}

	return manipulators;
}

float Animation::GetTicksPerSecond() const {
	return ticks_per_sec;
}

float Animation::GetDuration() const {
	return duration;
}

std::shared_ptr<AssimpNodeData> Animation::GetRootNode() const {
	return root_node;
}

std::unordered_map<std::string, BoneInfo> const& Animation::GetBoneInfoMap() const {
	return bone_info_map;
}

// HELPERS

void Animation::ReadMissingBones(aiAnimation const* anim, Model& model) {
	uint32_t size = anim->mNumChannels;

	std::unordered_map<std::string, BoneInfo>& bone_info_map_local = model.GetBoneInfoMap();
	int bone_count = model.GetBoneCount();

	for (uint32_t i = 0; i < size; ++i) {
		auto channel = anim->mChannels[i];
		std::string bone_name = channel->mNodeName.data;
		if (bone_info_map_local.find(bone_name) == bone_info_map_local.end()) {
			bone_info_map_local[bone_name].id = bone_count;
			model.SetBoneCount(++bone_count);
		}
		bones.push_back(std::make_shared<Bone>(channel->mNodeName.data, bone_info_map_local[channel->mNodeName.data].id, channel));
	}
	bone_info_map = bone_info_map_local;
}

// store the information in hierarchically stored nodes
std::shared_ptr<AssimpNodeData> Animation::ReadHierarchyData(std::shared_ptr<AssimpNodeData> parent, aiNode const* src) {
	assert(src);

	// update node data
	std::shared_ptr<AssimpNodeData> curr_node = std::make_shared<AssimpNodeData>();
	curr_node->name = src->mName.data;
	curr_node->transform = AssimpHelper::ConvertMatrixToGLMFormat(src->mTransformation);
	curr_node->parent = parent;
	curr_node->children_count = src->mNumChildren;
	curr_node->children.reserve(curr_node->children_count);

	// recursively read node data for children
	for (uint32_t i = 0; i < curr_node->children_count; ++i) {
		std::shared_ptr<AssimpNodeData> child_node = ReadHierarchyData(curr_node, src->mChildren[i]);
		curr_node->children.push_back(child_node);
	}

	return curr_node;
}

std::shared_ptr<AssimpNodeData> Animation::FindNodeData(std::string const& node_name) {
	std::queue<std::shared_ptr<AssimpNodeData>> bone_nodes{};
	bone_nodes.push(root_node);

	// start looking for node from root node
	while (not bone_nodes.empty()) {
		for (unsigned int i = 0; i < bone_nodes.size(); ++i) {
			std::shared_ptr<AssimpNodeData> curr_node = bone_nodes.front();
			bone_nodes.pop();

			if (curr_node->name == node_name) {
				return curr_node;
			}

			// push children to back of queue
			for (std::shared_ptr<AssimpNodeData> child : curr_node->children) {
				bone_nodes.push(child);
			}
		}
	}

	return nullptr;
}