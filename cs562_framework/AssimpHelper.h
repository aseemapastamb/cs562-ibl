#pragma once

#include "Quaternion.h"

// for conversions from assimp data types
class AssimpHelper {
public:
	static glm::mat4 ConvertMatrixToGLMFormat(aiMatrix4x4 const& aiMat);
	static glm::vec3 GetGLMVec3(aiVector3D const& aiVec3);
	static Quaternion GetQuat(aiQuaternion const& aiQuat);
};