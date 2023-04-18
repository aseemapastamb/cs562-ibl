#pragma once

// fwd decl
class PointMass;
class Spring;
class Mesh;

#include "Shader.h"

class Cloth {
public:
	Cloth();
	~Cloth();

	void Update(float dt);
	void Draw(Shader& shader);

	void SetAnchorPoints(glm::vec3 const& _ap1, glm::vec3 const& _ap2);
	void SetSphereCenter(glm::vec3 const& _sphere_center);

private:
	// sphere that this cloth collides with
	glm::vec3 sphere_center;
	float sphere_radius;
	Mesh* sphere_mesh;

	// anchor points
	PointMass* anchor1;
	PointMass* anchor2;
	glm::vec3 ap1, ap2;

	// all point masses
	std::vector<
		std::vector<
			std::vector<PointMass*>>> point_masses;
	// number of point masses
	unsigned num_x, num_z, num_y;

	// vector of all springs
	std::vector<Spring*> springs;
	// number of springs
	unsigned num_springs;

	// vector of springs to draw (only elastic and shear)
	std::vector<Spring*> draw_springs;
	// number of springs to draw
	unsigned num_draw_springs;

	// corner points
	glm::vec3 corner1, corner2, corner3, corner4, corner5, corner6, corner7, corner8;

	// drawing data
	GLuint vao, vbo, ebo;

private:
	void Setup(bool _cloth);
};