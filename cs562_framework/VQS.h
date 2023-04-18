#pragma once

#include "Quaternion.h"

class VQS {
public:
	glm::vec3 v;
	Quaternion q;
	float s;

	// ctors & dtors
	VQS();
	VQS(glm::vec3& _v, Quaternion& _q, float _s);
	VQS(VQS const& _t);
	VQS& operator= (VQS const& _t);
	~VQS() = default;

	// operations
	// add & subtract
	VQS operator+ (VQS const& _t) const;
	void operator+= (VQS const& _t);
	VQS operator- (VQS const& _t) const;
	void operator-= (VQS const& _t);
	// product
	// with scalar
	VQS operator* (float const& _s) const;
	void operator*= (float const& _s);
	// with VQS
	VQS operator* (VQS const& _t) const;
	VQS Multiply(VQS const& _t) const;
	void operator*= (VQS const& _t);
	// division
	// with scalar
	VQS operator/ (float const& _s) const;
	void operator/= (float const& _s);
	// inverse
	VQS Inverse() const;

	// rotation
	glm::vec3 operator* (glm::vec3 const& _r) const;
	glm::vec3 Multiply(glm::vec3 const& _r) const;
	glm::vec3 Rotate(glm::vec3 const& _r) const;
};

// first operand is scalar
VQS operator* (float _s, VQS const& _t);