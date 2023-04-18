#include "stdafx.h"

#include "Cloth.h"

#include "PointMass.h"
#include "Spring.h"
#include "Mesh.h"
#include "Texture.h"

Cloth::Cloth():
	sphere_center{ 0.0f, -3.0f, -3.0f },
	sphere_radius{ 5.0f },
	ap1{ -6.0f, 10.0f, -1.0f },
	ap2{ 6.0f, 10.0f, -1.0f },
	anchor1{ nullptr },
	anchor2{ nullptr },
	corner1{ -5.0f, 10.0f, -1.0f },
	corner2{ 5.0f, 10.0f, -1.0f },
	corner3{ 5.0f, 10.0f, 7.0f },
	corner4{ -5.0f, 10.0f, 7.0f },
	corner5{ -5.0f, 5.0f, -1.0f },
	corner6{ 5.0f, 5.0f, -1.0f },
	corner7{ 5.0f, 5.0f, 7.0f },
	corner8{ -5.0f, 5.0f, 7.0f },
	num_x{ 31 },
	num_z{ 31 },
	num_y{ 1 },
	num_springs{ 0 },
	num_draw_springs{ 0 }
{
	Setup(true);
}

Cloth::~Cloth() {
	delete anchor1;
	delete anchor2;
	for (unsigned i = 0; i < num_x; ++i) {
		for (unsigned j = 0; j < num_z; ++j) {
			for (unsigned k = 0; k < num_y; ++k) {
				delete point_masses[i][j][k];
			}
			point_masses[i][j].clear();
		}
		point_masses[i].clear();
	}
	point_masses.clear();
	for (unsigned i = 0; i < num_springs; ++i) {
		delete springs[i];
	}
	springs.clear();
	draw_springs.clear();
}

void Cloth::Update(float dt) {
	// divide timestep into 10 parts for more accuracy
	for (unsigned timestep = 0; timestep < 10; ++timestep) {
		for (unsigned i = 0; i < num_springs; ++i) {
			PointMass* pm_a = springs[i]->pm_a;
			PointMass* pm_b = springs[i]->pm_b;

			// A - B
			glm::vec3 A_minus_B = pm_a->position - pm_b->position;
			float length = glm::distance(pm_a->position, pm_b->position);

			// Dampers (friction)
			// damper coefficient of proportionality
			float damp_coeff = 0.5f;
			glm::vec3 damping_force1 = -damp_coeff * (pm_a->GetVel() - pm_b->GetVel());
			glm::vec3 damping_force2 = -damping_force1;

			// spring force (Hooke's Law)
			// F = -k * (|A - B| - l) * (A - B) / (|A - B|)
			glm::vec3 spring_force1 = -springs[i]->ks * (length - springs[i]->rest_length) * A_minus_B / length;	// on Point Mass 1
			glm::vec3 spring_force2 = -spring_force1;																// on Point Mass 2

			// gravity
			glm::vec3 grav{ 0.0f, -0.0098f, 0.0f };

			// apply forces
			if (pm_a != anchor1/* && pm_a != anchor2*/) {
				pm_a->ApplyForce(damping_force1 + spring_force1 + grav);
			}
			if (pm_b != anchor2/* && pm_b != anchor2*/) {
				pm_b->ApplyForce(damping_force2 + spring_force2 + grav);
			}
		}

		for (unsigned i = 0; i < num_x; ++i) {
			for (unsigned j = 0; j < num_z; ++j) {
				for (unsigned k = 0; k < num_y; ++k) {
					point_masses[i][j][k]->Update(dt * 0.1f);
				}
			}
		}
	}
}

void Cloth::Draw(Shader& shader) {
	// draw springs
	uint32_t spring_vao, spring_vbo;
	shader.SetVec3("aColor", glm::vec3{ 1.0f, 1.0f, 1.0f });
	glLineWidth(1.0f);

	glGenVertexArrays(1, &spring_vao);
	glBindVertexArray(spring_vao);

	glGenBuffers(1, &spring_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, spring_vbo);

	for (unsigned i = 0; i < num_draw_springs; ++i) {
		glm::vec3 p1 = draw_springs[i]->pm_a->position;
		glm::vec3 p2 = draw_springs[i]->pm_b->position;
		float p0[6]{
			p1.x,
			p1.y,
			p1.z,
			p2.x,
			p2.y,
			p2.z
		};
		glBufferData(GL_ARRAY_BUFFER, 6 * sizeof(float), &p0[0], GL_STREAM_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
		glDrawArrays(GL_LINES, 0, 2);
	}

	glBindVertexArray(0);
	glDeleteBuffers(1, &spring_vbo);
	glDeleteVertexArrays(1, &spring_vao);

	// draw point masses
	shader.SetVec3("aColor", glm::vec3{ 0.4f, 0.9f, 0.4f });
	anchor1->Draw(shader);
	anchor2->Draw(shader);
	shader.SetVec3("aColor", glm::vec3{ 0.46f, 0.53f, 0.6f });
	for (unsigned i = 0; i < num_x; ++i) {
		for (unsigned j = 0; j < num_z; ++j) {
			for (unsigned k = 0; k < num_y; ++k) {
				point_masses[i][j][k]->Draw(shader);
			}
		}
	}

	// draw sphere mesh
	glm::mat4 translate = glm::translate(glm::mat4(1.0f), sphere_center);
	glm::mat4 scale = glm::scale(glm::mat4(1.0f), glm::vec3(sphere_radius));
	glm::mat4 model = translate * scale;
	shader.SetMat4("model", model);
	shader.SetVec3("aColor", glm::vec3{ 0.2f, 0.2f, 0.2f });
	sphere_mesh->Draw(shader);
}

void Cloth::SetAnchorPoints(glm::vec3 const& _ap1, glm::vec3 const& _ap2) {
	anchor1->position = _ap1;
	anchor2->position = _ap2;
}

void Cloth::SetSphereCenter(glm::vec3 const& _sphere_center) {
	sphere_center = _sphere_center;

	for (unsigned i = 0; i < num_x; ++i) {
		for (unsigned j = 0; j < num_z; ++j) {
			for (unsigned k = 0; k < num_y; ++k) {
				point_masses[i][j][k]->SetupSphere(sphere_center, sphere_radius);
			}
		}
	}
}

// HELPERS

void Cloth::Setup(bool _cloth) {
	float x_len = corner2.x - corner1.x;
	x_len /= static_cast<float>(num_x);
	float z_len = corner4.z - corner1.z;
	z_len /= static_cast<float>(num_z);
	float y_len = corner5.y - corner1.y;
	y_len /= static_cast<float>(num_y);
	point_masses.resize(num_x);
	// x value
	for (unsigned i = 0; i < num_x; ++i) {
		point_masses[i].resize(num_z);
		// z value
		for (unsigned j = 0; j < num_z; ++j) {
			point_masses[i][j].resize(num_y);
			// y value
			for (unsigned k = 0; k < num_y; ++k) {
				// equally spaced point masses
				PointMass* pm = new PointMass{ glm::vec3{ corner1.x + x_len * i, corner1.y + y_len * k, corner1.z + z_len * j }, 0.03f };
				pm->SetupSphere(sphere_center, sphere_radius);
				point_masses[i][j][k] = pm;
			}
		}
	}

	// setup springs
	for (unsigned i = 0; i < num_x; ++i) {
		for (unsigned j = 0; j < num_z; ++j) {
			for (unsigned k = 0; k < num_y; ++k) {
				// elastic
				if (i + 1 < num_z) { // neighbour along x
					Spring* spring = new Spring{ point_masses[i][j][k], point_masses[i + 1][j][k] };
					springs.push_back(spring);
					draw_springs.push_back(spring);
				}
				if (j + 1 < num_z) { // neighbour along z
					Spring* spring = new Spring{ point_masses[i][j][k], point_masses[i][j + 1][k] };
					springs.push_back(spring);
					draw_springs.push_back(spring);
				}
				if (k + 1 < num_y) { // neighbour along y
					Spring* spring = new Spring{ point_masses[i][j][k], point_masses[i][j][k + 1] };
					springs.push_back(spring);
					draw_springs.push_back(spring);
				}

				// bend
				if (i + 2 < num_x) { // neighbour along x
					Spring* spring = new Spring{ point_masses[i][j][k], point_masses[i + 2][j][k] };
					springs.push_back(spring);
					//draw_springs.push_back(spring);
				}
				if (j + 2 < num_z) { // neighbour along z
					Spring* spring = new Spring{ point_masses[i][j][k], point_masses[i][j + 2][k] };
					springs.push_back(spring);
					//draw_springs.push_back(spring);
				}
				if (k + 2 < num_y) { // neighbour along y
					Spring* spring = new Spring{ point_masses[i][j][k], point_masses[i][j][k + 2] };
					springs.push_back(spring);
					//draw_springs.push_back(spring);
				}

				// shear
				if (i + 1 < num_x) { // diagonal neighbours, x-z plane
					if (j + 1 < num_z) { // next z
						Spring* spring = new Spring{ point_masses[i][j][k], point_masses[i + 1][j + 1][k] };
						springs.push_back(spring);
						draw_springs.push_back(spring);
						if (k + 1 < num_y) { // next y
							Spring* spring = new Spring{ point_masses[i][j][k], point_masses[i + 1][j + 1][k + 1] };
							springs.push_back(spring);
							draw_springs.push_back(spring);
						}
						if (k > 0) { // previous y
							Spring* spring = new Spring{ point_masses[i][j][k], point_masses[i + 1][j + 1][k - 1] };
							springs.push_back(spring);
							draw_springs.push_back(spring);
						}
					}
					if (j > 0) { // previous z
						Spring* spring = new Spring{ point_masses[i][j][k], point_masses[i + 1][j - 1][k] };
						springs.push_back(spring);
						draw_springs.push_back(spring);
						if (k + 1 < num_y) { // next y
							Spring* spring = new Spring{ point_masses[i][j][k], point_masses[i + 1][j - 1][k + 1] };
							springs.push_back(spring);
							draw_springs.push_back(spring);
						}
						if (k > 0) { // previous y
							Spring* spring = new Spring{ point_masses[i][j][k], point_masses[i + 1][j - 1][k - 1] };
							springs.push_back(spring);
							draw_springs.push_back(spring);
						}
					}
				}
				if (k + 1 < num_y) { // diagonal neighbours, x-y plane
					if (j + 1 < num_z) { // next z
						Spring* spring = new Spring{ point_masses[i][j][k], point_masses[i][j + 1][k + 1] };
						springs.push_back(spring);
						draw_springs.push_back(spring);
					}
					if (j > 0) { // previous z
						Spring* spring = new Spring{ point_masses[i][j][k], point_masses[i][j - 1][k + 1] };
						springs.push_back(spring);
						draw_springs.push_back(spring);
					}
				}
			}
		}
	}

	// anchor points
	anchor1 = new PointMass{ ap1, 10.0f };
	anchor2 = new PointMass{ ap2, 10.0f };

	// spring connections to anchor points
	Spring* spr1 = new Spring{ anchor1, point_masses[0][0][0] };
	Spring* spr2{ nullptr };
	if (_cloth) {
		spr2 = new Spring{ point_masses[num_x - 1][0][0], anchor2 };
	}
	else {
		spr2 = new Spring{ point_masses[num_x - 1][num_z - 1][0], anchor2 };
	}
	springs.push_back(spr1);
	draw_springs.push_back(spr1);
	springs.push_back(spr2);
	draw_springs.push_back(spr2);

	num_springs = static_cast<unsigned>(springs.size());
	num_draw_springs = static_cast<unsigned>(draw_springs.size());

	// sphere mesh
	std::vector<Vertex> vertices;
	std::vector<uint32_t> indices;
	std::vector<Texture*> textures;

	// Sphere
	const unsigned int X_SEGMENTS = 64;
	const unsigned int Y_SEGMENTS = 64;
	const float PI = 3.14159265359f;
	for (unsigned int x = 0; x <= X_SEGMENTS; ++x)
	{
		for (unsigned int y = 0; y <= Y_SEGMENTS; ++y)
		{
			Vertex vertex;

			float xSegment = (float)x / (float)X_SEGMENTS;
			float ySegment = (float)y / (float)Y_SEGMENTS;
			float xPos = std::cos(xSegment * 2.0f * PI) * std::sin(ySegment * PI);
			float yPos = std::cos(ySegment * PI);
			float zPos = std::sin(xSegment * 2.0f * PI) * std::sin(ySegment * PI);

			// position
			vertex.position = glm::vec3(xPos, yPos, zPos);
			// normal
			vertex.normal = glm::vec3(xPos, yPos, zPos);
			// uv
			vertex.tex_coords = glm::vec2(xSegment, ySegment);

			vertices.push_back(vertex);
		}
	}

	bool oddRow = false;
	for (unsigned int y = 0; y < Y_SEGMENTS; ++y)
	{
		if (!oddRow) // even rows: y == 0, y == 2; and so on
		{
			for (unsigned int x = 0; x <= X_SEGMENTS; ++x)
			{
				indices.push_back(y * (X_SEGMENTS + 1) + x);
				indices.push_back((y + 1) * (X_SEGMENTS + 1) + x);
			}
		}
		else
		{
			for (int x = X_SEGMENTS; x >= 0; --x)
			{
				indices.push_back((y + 1) * (X_SEGMENTS + 1) + x);
				indices.push_back(y * (X_SEGMENTS + 1) + x);
			}
		}
		oddRow = !oddRow;
	}

	sphere_mesh = new Mesh{ vertices, indices, textures };
}
