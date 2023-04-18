#pragma once

// forward declarations
class Animation;
struct AssimpNodeData;

class Animator {
public:
	Animator();

	void UpdateAnimation(float dt);
	// load new animation
	void PlayAnimation(std::shared_ptr<Animation> p_anim);
	// update values of each bone
	void CalculateBoneTransform(std::shared_ptr<AssimpNodeData> node, glm::mat4 parent_transform, bool ik_update);

	// get list of manipulators from animation
	void SetupIK(std::string const& end_effector);
	void ResetIK();
	void SolveIK(glm::vec3 const& target_pos, float dt);

	std::vector<glm::mat4> const& GetFinalBoneMatrices() const;
	std::vector<std::tuple<uint32_t, glm::vec3>> const& GetBonePositions() const;

public:
	// current animation
	std::shared_ptr<Animation> curr_anim;

private:
	// final transforms of each bone
	std::vector<glm::mat4> final_bone_matrices;
	// position of bones along with "set" number
	std::vector<std::tuple<uint32_t, glm::vec3>> bone_positions;
	// to keep track of bone "sets" - needed to remove unnecessary lines between bones
	uint32_t set_counter;

	float curr_time;

	// vector of manipulators from end effector to root node
	std::vector<std::shared_ptr<AssimpNodeData>> manipulators;

	// vector of rotation angle constraints
	std::vector<glm::vec2> constraints;

	// vector of angle currently rotated to reach target
	std::vector<float> angle_rotated;

private:

};