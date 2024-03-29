#include "stdafx.h"

#include "AssimpHelper.h"

glm::mat4 AssimpHelper::ConvertMatrixToGLMFormat(aiMatrix4x4 const& aiMat) {
	glm::mat4 mat;
	mat[0][0] = aiMat.a1; mat[1][0] = aiMat.a2; mat[2][0] = aiMat.a3; mat[3][0] = aiMat.a4;
	mat[0][1] = aiMat.b1; mat[1][1] = aiMat.b2; mat[2][1] = aiMat.b3; mat[3][1] = aiMat.b4;
	mat[0][2] = aiMat.c1; mat[1][2] = aiMat.c2; mat[2][2] = aiMat.c3; mat[3][2] = aiMat.c4;
	mat[0][3] = aiMat.d1; mat[1][3] = aiMat.d2; mat[2][3] = aiMat.d3; mat[3][3] = aiMat.d4;
	return mat;
}

glm::vec3 AssimpHelper::GetGLMVec3(aiVector3D const& aiVec3) {
	return glm::vec3{ aiVec3.x, aiVec3.y, aiVec3.z };
}

Quaternion AssimpHelper::GetQuat(aiQuaternion const& aiQuat) {
	return Quaternion{ aiQuat.w, aiQuat.x, aiQuat.y, aiQuat.z };
}
