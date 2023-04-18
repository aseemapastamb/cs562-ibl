#pragma once

class FBO {
public:
	uint32_t fbo_id;
	uint32_t texture_id[4] = { 0, 0, 0, 0 };
	uint32_t depth_buffer_id;

public:
	/// <summary>
	/// Constructor for FBO object
	/// </summary>
	/// <param name="w">Width</param>
	/// <param name="h">Height</param>
	/// <param name="_colour_attachment_count">Number of colour attachments</param>
	/// <param name="_has_depth_buffer">True if z-tests needed on buffer</param>
	FBO(uint32_t const w, uint32_t const h, uint32_t const _colour_attachment_count = 1, bool _has_depth_buffer = true);
	/// <summary>
	/// Destructor for FBO.
	/// Deletes associated texture.
	/// </summary>
	~FBO();

	/// <summary>
	/// Bind FBO to use it as render target in next draw call of graphics pipeline
	/// </summary>
	void Bind() const;
	/// <summary>
	/// Unbind FBO so graphics pipeline draws to screen by default
	/// </summary>
	void Unbind() const;

	/// <summary>
	/// Bind FBO's texture to a texture unit
	/// </summary>
	/// <param name="program_id"></param>
	/// <param name="texture_name"></param>
	/// <param name="colour_attachment"></param>
	/// <param name="texture_unit"></param>
	void BindTexture(uint8_t program_id, const char* texture_name, uint32_t const colour_attachment, uint32_t const texture_unit) const;
	/// <summary>
	/// Unbind FBO's texture from texture unit
	/// </summary>
	/// <param name="colour_attachment"></param>
	/// <param name="texture_unit"></param>
	void UnbindTexture(uint32_t const texture_unit) const;

	void BindReadImageTexture(uint8_t program_id, const char* image_texture_name, uint32_t const colour_attachment, uint32_t const image_unit) const;
	void BindWriteImageTexture(uint8_t program_id, const char* image_name, uint32_t const colour_attachment, uint32_t const image_unit) const;

	// Getters
	inline uint32_t GetWidth() const { return width; }
	inline uint32_t GetHeight() const { return height; }

	// Setters
	void SetWidth(uint32_t _width) { width = _width; }
	void SetHeight(uint32_t _height) { height = _height; }

private:
	uint32_t width, height;
	uint32_t colour_attachment_count;
	bool has_depth_buffer;
	
};