#pragma once

#include "Bone.h"
#include "AnimationData.h"
#include "Model.h"

// forward declarations
class Model;

// data structure to hierarchically store node data
struct AssimpNodeData {
	glm::mat4 transform;
	std::string name;
	uint32_t children_count;
	std::shared_ptr<AssimpNodeData> parent;
	std::vector<std::shared_ptr<AssimpNodeData>> children;

	AssimpNodeData():
		transform{},
		name{},
		children_count{},
		parent{ nullptr } {

	}
};

class Animation {
private:
	float duration;
	float ticks_per_sec;
	std::shared_ptr<AssimpNodeData> root_node;
	std::unordered_map<std::string, BoneInfo> bone_info_map;

private:
	std::shared_ptr<AssimpNodeData> FindNodeData(std::string const& node_name);

	void ReadMissingBones(aiAnimation const* anim, Model& model);
	std::shared_ptr<AssimpNodeData> ReadHierarchyData(std::shared_ptr<AssimpNodeData> parent, aiNode const* src);

public:
	std::vector<std::shared_ptr<Bone>> bones;
	std::string m_anim_path;

public:
	Animation();
	~Animation() = default;

	void LoadAnimation(std::string const& anim_path, Model* model);

	std::vector<std::shared_ptr<AssimpNodeData>> GetManipulators(std::string const& end_effector_name);

	std::shared_ptr<Bone> FindBone(std::string const& name);

	float GetTicksPerSecond() const;
	float GetDuration() const;
	std::shared_ptr<AssimpNodeData> GetRootNode() const;
	std::unordered_map<std::string, BoneInfo> const& GetBoneInfoMap() const;
};