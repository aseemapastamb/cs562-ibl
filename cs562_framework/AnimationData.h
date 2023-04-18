#pragma once

struct BoneInfo {
	// index in final_bone_matrices
	int id;
	// mat to convert vertex from model space to bone space
	glm::mat4 offset;
};