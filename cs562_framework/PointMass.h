#pragma once

#include "Mesh.h"

class PointMass {
public:
	float mass;
	glm::vec3 position;

public:
	PointMass(glm::vec3 _pos, float _mass);
	~PointMass() = default;

	void SetupSphere(glm::vec3 _pos, float _radius);

	void Update(float dt);

	void ApplyForce(glm::vec3 _force);
	void ResolveCollision();

	void Draw(Shader& shader);

	inline glm::vec3 GetVel() const { return linear_vel; }

private:
	glm::vec3 linear_vel;
	glm::vec3 linear_force;
	float inverse_mass;

	Mesh* mesh;

	// collide with this sphere
	glm::vec3 sphere_center;
	float sphere_radius;

private:
	void SetupMesh();
};

struct Data {
	glm::vec3 dx;
	glm::vec3 dv;

	Data():
		dx{ 0.0f },
		dv{ 0.0f }
	{}
	~Data() = default;
};