#pragma once

class Shader {
private:
	// Utility function to check shader compilation and linking errors
	void CheckCompileErrors(uint32_t shader, std::string type);

public:
	Shader();
	Shader(const char* compute_path);
	Shader(const char* vertex_path, const char* fragment_path);
	~Shader();

	// Activating shader
	void Use();
	void Unuse();

	// Utility
	void SetBool(const std::string& name, bool value) const;
	void SetUInt(const std::string& name, unsigned int value) const;
	void SetInt(const std::string& name, int value) const;
	void SetFloat(const std::string& name, float value) const;
	void SetIVec2(const std::string& name, const glm::ivec2& value) const;
	void SetVec2(const std::string& name, const glm::vec2& value) const;
	void SetVec2(const std::string& name, float x, float y) const;
	void SetVec3(const std::string& name, const glm::vec3& value) const;
	void SetVec3(const std::string& name, float x, float y, float z) const;
	void SetVec4(const std::string& name, const glm::vec4& value) const;
	void SetVec4(const std::string& name, float x, float y, float z, float w) const;
	void SetMat2(const std::string& name, const glm::mat2& mat) const;
	void SetMat3(const std::string& name, const glm::mat3& mat) const;
	void SetMat4(const std::string& name, const glm::mat4& mat) const;

	uint32_t id;
};