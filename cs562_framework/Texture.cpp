#include "stdafx.h"

#include "Texture.h"

TextureHDR::TextureHDR(const char* texture_path):
	tex_id{ 0 }
{
	int width, height, num_comp;

	stbi_set_flip_vertically_on_load(false);

	float* image = stbi_loadf(texture_path, &width, &height, &num_comp, 0);
	if (!image) {
		std::cout << "Error reading hdr file: " << texture_path << std::endl;
		std::cout << "Error: " << stbi_failure_reason() << std::endl;
	}

	glGenTextures(1, &tex_id);
	glBindTexture(GL_TEXTURE_2D, tex_id);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_FLOAT, image);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 10);
	glGenerateMipmap(GL_TEXTURE_2D);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, (int)GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, (int)GL_LINEAR_MIPMAP_LINEAR);
	glBindTexture(GL_TEXTURE_2D, 0);

	stbi_image_free(image);
}

TextureHDR::~TextureHDR()
{
}

void TextureHDR::Bind(uint8_t shader_program_id, uint32_t texture_unit, const char* texture_name) {
	//int loc = glGetUniformLocation(shader_program_id, texture_name);
	//glUniform1i(loc, texture_unit);
	glActiveTexture((GLenum)(int)GL_TEXTURE0 + texture_unit);
	glBindTexture(GL_TEXTURE_2D, tex_id);
}

void TextureHDR::Unbind() {
	glBindTexture(GL_TEXTURE_2D, 0);
}

Texture::Texture(const char* texture_path):
	tex_id{ 0 },
	path{ texture_path }
{
	int width, height, num_comp;

	stbi_set_flip_vertically_on_load(true);

	unsigned char* image = stbi_load(texture_path, &width, &height, &num_comp, 4);
	if (!image) {
		std::cout << "Error reading file: " << texture_path << std::endl;
		std::cout << "Error: " << stbi_failure_reason() << std::endl;
	}

	glGenTextures(1, &tex_id);
	glBindTexture(GL_TEXTURE_2D, tex_id);

	// set format from number of components
	GLint mode = GL_SRGB;
	GLenum format = 0;
	switch (num_comp) {
	case 0:
		format = GL_RED;
		break;
	case 3:
		format = GL_RGB;
		break;
	case 4:
		mode = GL_SRGB_ALPHA;
		format = GL_RGBA;
		break;
	default:
		break;
	}
	glTexImage2D(GL_TEXTURE_2D, 0, (GLint)format, width, height, 0, format, GL_UNSIGNED_BYTE, image);
	glGenerateMipmap(GL_TEXTURE_2D);

	// wrapping parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	// filter parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, (int)GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, (int)GL_LINEAR_MIPMAP_LINEAR);

	// unbind
	glBindTexture(GL_TEXTURE_2D, 0);

	stbi_image_free(image);
}

Texture::~Texture() {
	glDeleteTextures(1, &tex_id);
}

void Texture::Bind(uint8_t shader_program_id, uint32_t texture_unit, const char* texture_name) {
	//int loc = glGetUniformLocation(shader_program_id, texture_name);
	//glUniform1i(loc, texture_unit);
	glActiveTexture((GLenum)(int)GL_TEXTURE0 + texture_unit);
	glBindTexture(GL_TEXTURE_2D, tex_id);
}

void Texture::Unbind() {
	glBindTexture(GL_TEXTURE_2D, 0);
}

void Texture::GenerateDefaultMipMap() {
	glBindTexture(GL_TEXTURE_2D, tex_id);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 10);
	glGenerateMipmap(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, 0);
}
