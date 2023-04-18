#include "stdafx.h"

#include "PointMass.h"

constexpr auto MAX_FLOAT = 3.402823466e+38F;

PointMass::PointMass(glm::vec3 _pos, float _mass):
	mass{ _mass },
	position{ _pos },
	linear_vel{ 0.0f },
	linear_force{ 0.0f },
	sphere_center{ 0.0f },
	sphere_radius{ 0.0f },
	inverse_mass{},
	mesh{ nullptr }
{
	if (mass > 0.0f) {
		inverse_mass = 1.0f / mass;
	}
	else {
		inverse_mass = MAX_FLOAT;
	}
	SetupMesh();
}

void PointMass::SetupSphere(glm::vec3 _pos, float _radius) {
	sphere_center = _pos;
	sphere_radius = _radius;
}

void PointMass::Update(float dt) {
	//// Euler
	//linear_vel += linear_force * dt * inverse_mass;
	//position += linear_vel * dt;

	// Runge-Kutta 4th order (RK-4)
	Data d, k1, k2, k3, k4;
	glm::vec3 acc = linear_force * inverse_mass;

	// beginning of timestep = d.dv = 0
	k1.dx = linear_vel + d.dv * dt;
	k1.dv = acc;
	// half timestep
	k2.dx = linear_vel + k1.dv * dt * 0.5f;
	k2.dv = acc;
	// half timestep
	k3.dx = linear_vel + k2.dv * dt * 0.5f;
	k3.dv = acc;
	// complete timestep
	k4.dx = linear_vel + k3.dv * dt;
	k4.dv = acc;

	position += (k1.dx + 2.0f * k2.dx + 2.0f * k3.dx + k4.dx) * dt / 6.0f;
	linear_vel += (k1.dv + 2.0f * k2.dv + 2.0f * k3.dv + k4.dv) * dt / 6.0f;

	// resetting force
	linear_force = glm::vec3{ 0.0f };

	// with big sphere
	ResolveCollision();

	// ground
	if (position.y < 0.0f) {
		//linear_vel.y *= -1.0f;
		position.y = 0.0f;
	}
}

void PointMass::ApplyForce(glm::vec3 _force) {
	linear_force += _force;
}

// collision detection and resolution
void PointMass::ResolveCollision() {
	glm::vec3 dir = position - sphere_center;
	float dist = glm::length(dir);
	if (dist < sphere_radius) {
		// colliding
		glm::vec3 offset = glm::normalize(dir) * (sphere_radius - dist);
		position += offset;
	}
}

void PointMass::Draw(Shader& shader) {
	glm::mat4 translate = glm::translate(glm::mat4(1.0f), position);
	glm::mat4 scale = glm::scale(glm::mat4(1.0f), glm::vec3(0.03f));
	glm::mat4 model = translate * scale;
	shader.SetMat4("model", model);
	mesh->Draw(shader);
}

// HELPERS

void PointMass::SetupMesh() {
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

	mesh = new Mesh{ vertices, indices, textures };
}
