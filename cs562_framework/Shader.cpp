#include "stdafx.h"

#include <fstream>
#include <sstream>

#include "Shader.h"

Shader::Shader():
	id{ 0 }
{

}

Shader::Shader(const char* compute_path) {
	// Retrieving shader source code from files
	std::string compute_code;
	std::ifstream c_shader_file;

	c_shader_file.exceptions(std::ifstream::failbit | std::ifstream::badbit);
	try {
		c_shader_file.open(compute_path);
		std::stringstream c_shader_stream;

		// read file buffer into stream
		c_shader_stream << c_shader_file.rdbuf();

		c_shader_file.close();

		// convert stream to string
		compute_code = c_shader_stream.str();
	}
	catch (std::ifstream::failure& e) {
		std::cout << "Error reading shader file: " << e.what() << std::endl;
	}

	const char* c_shader_code = compute_code.c_str();

	// create and compile shader
	uint32_t compute = glCreateShader(GL_COMPUTE_SHADER);
	glShaderSource(compute, 1, &c_shader_code, NULL);
	glCompileShader(compute);
	CheckCompileErrors(compute, "COMPUTE");

	// create shader program
	id = glCreateProgram();
	glAttachShader(id, compute);
	glLinkProgram(id);
	CheckCompileErrors(id, "PROGRAM");

	// Delete linked shader
	glDeleteShader(compute);
}

Shader::Shader(const char* vertex_path, const char* fragment_path) {
	// Retrieving shader source code from files
	std::string vertex_code;
	std::string fragment_code;
	std::ifstream v_shader_file;
	std::ifstream f_shader_file;

	v_shader_file.exceptions(std::ifstream::failbit | std::ifstream::badbit);
	f_shader_file.exceptions(std::ifstream::failbit | std::ifstream::badbit);
	try {
		v_shader_file.open(vertex_path);
		f_shader_file.open(fragment_path);
		std::stringstream v_shader_stream;
		std::stringstream f_shader_stream;

		// read file buffer into stream
		v_shader_stream << v_shader_file.rdbuf();
		f_shader_stream << f_shader_file.rdbuf();

		v_shader_file.close();
		f_shader_file.close();

		// convert stream to string
		vertex_code = v_shader_stream.str();
		fragment_code = f_shader_stream.str();
	}
	catch (std::ifstream::failure& e) {
		std::cout << "Error reading shader file: " << e.what() << std::endl;
	}

	const char* v_shader_code = vertex_code.c_str();
	const char* f_shader_code = fragment_code.c_str();

	// Compiling shader
	uint32_t vertex = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertex, 1, &v_shader_code, NULL);
	glCompileShader(vertex);
	CheckCompileErrors(vertex, "VERTEX");
	uint32_t fragment = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragment, 1, &f_shader_code, NULL);
	glCompileShader(fragment);
	CheckCompileErrors(fragment, "FRAGMENT");

	// Shader program
	id = glCreateProgram();
	glAttachShader(id, vertex);
	glAttachShader(id, fragment);
	glLinkProgram(id);
	CheckCompileErrors(id, "PROGRAM");

	// Delete linked shaders
	glDeleteShader(vertex);
	glDeleteShader(fragment);
}

Shader::~Shader() {

}

// Activating shader
void Shader::Use() {
	glUseProgram(id);
}

void Shader::Unuse() {
	glUseProgram(0);
}

void Shader::SetBool(const std::string& name, bool value) const {
	glUniform1i(glGetUniformLocation(id, name.c_str()), (int)value);
}

void Shader::SetUInt(const std::string& name, unsigned int value) const {
	glUniform1ui(glGetUniformLocation(id, name.c_str()), value);
}

void Shader::SetInt(const std::string& name, int value) const {
	glUniform1i(glGetUniformLocation(id, name.c_str()), value);
}

void Shader::SetFloat(const std::string& name, float value) const {
	glUniform1f(glGetUniformLocation(id, name.c_str()), value);
}

void Shader::SetIVec2(const std::string& name, const glm::ivec2& value) const {
	glUniform2iv(glGetUniformLocation(id, name.c_str()), 1, &value[0]);
}

void Shader::SetVec2(const std::string& name, const glm::vec2& value) const {
	glUniform2fv(glGetUniformLocation(id, name.c_str()), 1, &value[0]);
}

void Shader::SetVec2(const std::string& name, float x, float y) const {
	glUniform2f(glGetUniformLocation(id, name.c_str()), x, y);
}

void Shader::SetVec3(const std::string& name, const glm::vec3& value) const {
	glUniform3fv(glGetUniformLocation(id, name.c_str()), 1, &value[0]);
}

void Shader::SetVec3(const std::string& name, float x, float y, float z) const {
	glUniform3f(glGetUniformLocation(id, name.c_str()), x, y, z);
}

void Shader::SetVec4(const std::string& name, const glm::vec4& value) const {
	glUniform4fv(glGetUniformLocation(id, name.c_str()), 1, &value[0]);
}

void Shader::SetVec4(const std::string& name, float x, float y, float z, float w) const {
	glUniform4f(glGetUniformLocation(id, name.c_str()), x, y, z, w);
}

void Shader::SetMat2(const std::string& name, const glm::mat2& mat) const {
	glUniformMatrix2fv(glGetUniformLocation(id, name.c_str()), 1, GL_FALSE, &mat[0][0]);
}

void Shader::SetMat3(const std::string& name, const glm::mat3& mat) const {
	glUniformMatrix3fv(glGetUniformLocation(id, name.c_str()), 1, GL_FALSE, &mat[0][0]);
}

void Shader::SetMat4(const std::string& name, const glm::mat4& mat) const {
	glUniformMatrix4fv(glGetUniformLocation(id, name.c_str()), 1, GL_FALSE, &mat[0][0]);
}

// HELPERS

// Utility function to check shader compilation and linking errors
void Shader::CheckCompileErrors(uint32_t shader, std::string type) {
	int success;
	char info_log[1024];

	if (type != "PROGRAM") {
		glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
		if (!success) {
			glGetShaderInfoLog(shader, 1024, NULL, info_log);
			std::cout << "Shader compiling error of type [" << type << "] : " << info_log << std::endl;
		}
	}
	else {
		glGetProgramiv(shader, GL_LINK_STATUS, &success);
		if (!success) {
			glGetProgramInfoLog(shader, 1024, NULL, info_log);
			std::cout << "Linking error of type [" << type << "] : " << info_log << std::endl;
		}
	}
}