#pragma once

class TextureHDR {
public:
	uint32_t tex_id;
public:
	TextureHDR(const char* texture_path);
	~TextureHDR();

	void Bind(uint8_t shader_program_id, uint32_t texture_unit, const char* texture_name);
	void Unbind();
};

class Texture {
public:
	uint32_t tex_id;
	const char* path;

public:
	Texture(const char* texture_path);
	~Texture();

	void Bind(uint8_t shader_program_id, uint32_t texture_unit, const char* texture_name);
	void Unbind();

	void GenerateDefaultMipMap();
};