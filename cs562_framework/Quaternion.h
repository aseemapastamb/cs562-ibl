#pragma once

class Quaternion {
public:
	// scalar
	float s;
	// vector
	glm::vec3 v;

	// ctors & dtors
	Quaternion();
	Quaternion(float _s, glm::vec3& _v);
	Quaternion(float _s, float _x, float _y, float _z);
	Quaternion(Quaternion const& _q);
	Quaternion& operator= (Quaternion const& _q);
	~Quaternion() = default;

	// operations
	// add & subtract
	Quaternion operator+ (Quaternion const& _q) const;
	void operator+= (Quaternion const& _q);
	Quaternion operator- (Quaternion const& _q) const;
	void operator-= (Quaternion const& _q);
	// product
	// with scalar
	Quaternion operator* (float const& _s) const;
	void operator*= (float const& _s);
	// with Quaternion
	Quaternion operator* (Quaternion const& _q) const;
	Quaternion Multiply(Quaternion const& _q) const;
	void operator*= (Quaternion const& _q);
	// division
	// with scalar
	Quaternion operator/ (float const& _s) const;
	void operator/= (float const& _s);

	// dot
	float Dot(Quaternion const& _q) const;
	// magnitude
	float Magnitude() const;
	// normalize
	void Normalize();
	// conjugate
	Quaternion Conjugate() const;
	// inverse
	Quaternion Inverse() const;

	// conversions
	glm::mat3 ToMat3() const;

	// rotation
	glm::vec3 operator* (glm::vec3 const& _r) const;
	glm::vec3 Multiply(glm::vec3 const& _r) const;
	glm::vec3 Rotate(glm::vec3 const& _r) const;
};

// first operand is scalar
Quaternion operator* (float _s, Quaternion const& _q);