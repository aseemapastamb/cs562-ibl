#include "stdafx.h"

#include "FBO.h"

FBO::FBO(uint32_t const w, uint32_t const h, uint32_t const _colour_attachment_count, bool _has_depth_buffer):
	fbo_id{ 0 },
	depth_buffer_id{ 0 },
	width{ w },
	height{ h },
	colour_attachment_count{ _colour_attachment_count },
	has_depth_buffer{ _has_depth_buffer }
{
	glGenFramebuffersEXT(1, &fbo_id);
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fbo_id);

	if (has_depth_buffer) {
		// create a render buffer and attach it to FBO's depth attachment
		glGenRenderbuffersEXT(1, &depth_buffer_id);
		glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, depth_buffer_id);
		glRenderbufferStorageEXT(GL_RENDERBUFFER_EXT, GL_DEPTH_COMPONENT, width, height);
		glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT,
			GL_RENDERBUFFER_EXT, depth_buffer_id);
	}

	// Maximum colour attachments = 4
	assert(colour_attachment_count <= 4 && "More than maximum colour attachments");

	std::vector<uint32_t> colour_attachments;

	// Create a texture and attach FBO's colour attachments.
	// The GL_RGBA32F and GL_RGBA constants set this texture to be 32 bit
	// floats for each of the 4 components. Many other choices are
	// possible.
	for (uint8_t i = 0; i < colour_attachment_count; ++i) {
		glGenTextures(1, &texture_id[i]);
		glBindTexture(GL_TEXTURE_2D, texture_id[i]);
		glTexImage2D(GL_TEXTURE_2D, 0, (GLint)GL_RGBA32F, width, height, 0, GL_RGBA, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);

		// wrapping parameters
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, (int)GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, (int)GL_CLAMP_TO_EDGE);
		// filtering parameters
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, (int)GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, (int)GL_LINEAR);

		glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, (GLenum)((int)GL_COLOR_ATTACHMENT0_EXT + i),
			GL_TEXTURE_2D, texture_id[i], 0);

		colour_attachments.push_back(GL_COLOR_ATTACHMENT0_EXT + i);
	}
	glDrawBuffers(colour_attachment_count, colour_attachments.data());

	// Check for completeness/correctness
	int status = (int)glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
	assert(status == int(GL_FRAMEBUFFER_COMPLETE_EXT) && "FBO Creation error");
	
	// Unbind the fbo until it's ready to be used
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
}

FBO::~FBO() {
	if (fbo_id == 0)
		return;

	glDeleteTextures(colour_attachment_count, texture_id);
	glDeleteRenderbuffers(1, &depth_buffer_id);
	glDeleteFramebuffers(1, &fbo_id);
}

void FBO::Bind() const {
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fbo_id);
	glViewport(0, 0, width, height);
}

void FBO::Unbind() const {
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
}

void FBO::BindTexture(uint8_t program_id, const char* texture_name, uint32_t const colour_attachment, uint32_t const texture_unit) const {
	//int loc = glGetUniformLocation(program_id, texture_name);
	//glUniform1i(loc, texture_unit);
	glActiveTexture((GLenum)(int)GL_TEXTURE0 + texture_unit);
	glBindTexture(GL_TEXTURE_2D, texture_id[colour_attachment]);
}

void FBO::UnbindTexture(uint32_t const texture_unit) const {
	glActiveTexture((GLenum)(int)(GL_TEXTURE0 + texture_unit));
	glBindTexture(GL_TEXTURE_2D, 0);
}

void FBO::BindReadImageTexture(uint8_t program_id, const char* image_name, uint32_t const colour_attachment, uint32_t const image_unit) const {
	int loc = glGetUniformLocation(program_id, image_name);
	glUniform1i(loc, image_unit);
	//glActiveTexture((GLenum)(int)(GL_TEXTURE0 + image_unit));
	glBindImageTexture(image_unit, texture_id[colour_attachment], 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);
}

void FBO::BindWriteImageTexture(uint8_t program_id, const char* image_name, uint32_t const colour_attachment, uint32_t const image_unit) const {
	int loc = glGetUniformLocation(program_id, image_name);
	glUniform1i(loc, image_unit);
	//glActiveTexture((GLenum)(int)(GL_TEXTURE0 + image_unit));
	glBindImageTexture(image_unit, texture_id[colour_attachment], 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
}