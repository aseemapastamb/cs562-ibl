#pragma once

#include "Shader.h"

constexpr int MAX_BONE_INFLUENCE = 4;

struct Vertex {
	glm::vec3 position;
	glm::vec3 normal;
	glm::vec2 tex_coords;
	glm::vec3 tangent;
	glm::vec3 bi_tangent;

	// bone indexes which influence this vertex
	int bone_ids[MAX_BONE_INFLUENCE];
	// weights from each bone
	float weights[MAX_BONE_INFLUENCE];
};

//struct Texture {
//	uint32_t id;
//	std::string type;
//	std::string path;
//};
class Texture;

enum class DrawMode {
	POINTS,
	LINE_STRIP, 
	LINE_LOOP, 
	LINES, 
	LINE_STRIP_ADJACENCY, 
	LINES_ADJACENCY, 
	TRIANGLE_STRIP, 
	TRIANGLE_FAN, 
	TRIANGLES, 
	TRIANGLE_STRIP_ADJACENCY, 
	TRIANGLES_ADJACENCY, 
	PATCHES
};

class Mesh {
private:
	uint32_t vbo, ebo;

	void SetupMesh();

public:
	// mesh data
	std::vector<Vertex> vertices;
	std::vector<uint32_t> indices;
	std::vector<Texture*> textures;
	uint32_t vao;

	Mesh(std::vector<Vertex> vertices, std::vector<uint32_t> indices, std::vector<Texture*> textures);

	void Draw(Shader& shader, DrawMode draw_mode = DrawMode::TRIANGLES);
};