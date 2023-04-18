#pragma once

#include "VQS.h"

class Bone {
public:
	Bone(std::string const& name, int id, aiNodeAnim* channel);

	void Update();

	glm::mat4 GetLocalTransform() const;
	void SetLocalTransform(glm::mat4 new_local_transform);
	std::string const& GetBoneName() const;
	int GetBoneID() const;

	void SetNumIncr(int _num_incr);

private:
	uint32_t num_key_frames;

	// bone data
	glm::mat4 local_transform;
	std::string name;
	int id;

	// incremental VQS preprocessed info
	std::vector<VQS> VQS_c;
	// number of steps between frames
	int num_incr;
	// index of current iteration within keyframe
	int itr;
	// index of keyframe within animation loop
	int keyframe_index;

	// current vqs
	VQS curr_vqs;
	// prev vqs
	VQS prev_vqs;
	// all bone key frames data
	std::vector<VQS> vqs_key_frames;
};