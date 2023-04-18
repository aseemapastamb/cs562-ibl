#include "stdafx.h"

#include "Quaternion.h"

Quaternion::Quaternion() :
	s{ 0.0f },
	v{ 0.0f } {

}

Quaternion::Quaternion(float _s, glm::vec3& _v) :
	s{ _s },
	v{ _v } {

}


Quaternion::Quaternion(float _s, float _x, float _y, float _z) :
	s{ _s },
	v{ _x, _y, _z } {

}

Quaternion::Quaternion(Quaternion const& _q) :
	s{ _q.s },
	v{ _q.v } {

}

Quaternion& Quaternion::operator=(Quaternion const& _q) {
	s = _q.s;
	v = _q.v;
	return *this;
}

Quaternion Quaternion::operator+(Quaternion const& _q) const {
	float scalar = s + _q.s;
	glm::vec3 vector = v + _q.v;
	return Quaternion{ scalar, vector };
}

void Quaternion::operator+=(Quaternion const& _q) {
	s += _q.s;
	v += _q.v;
}

Quaternion Quaternion::operator-(Quaternion const& _q) const {
	float scalar = s - _q.s;
	glm::vec3 vector = v - _q.v;
	return Quaternion{ scalar, vector };
}

void Quaternion::operator-=(Quaternion const& _q) {
	s -= _q.s;
	v -= _q.v;
}

Quaternion Quaternion::operator*(float const& _s) const {
	float scalar = s * _s;
	glm::vec3 vector = v * _s;
	return Quaternion{ scalar, vector };
}

void Quaternion::operator*=(float const& _s) {
	s *= _s;
	v *= _s;
}

// non-class function
// first operator is scalar, so swap
Quaternion operator*(float _s, Quaternion const& _q) {
	return _q * _s;
}

Quaternion Quaternion::operator/(float const& _s) const {
	float scalar = s / _s;
	glm::vec3 vector = v / _s;
	return Quaternion{ scalar, vector };
}

void Quaternion::operator/=(float const& _s) {
	s /= _s;
	v /= _s;
}

Quaternion Quaternion::operator*(Quaternion const& _q) const {
	float scalar = s * _q.s - glm::dot(v, _q.v);
	glm::vec3 vector = (_q.v * s) + (v * _q.s) + glm::cross(v, _q.v);
	return Quaternion{ scalar, vector };
}

Quaternion Quaternion::Multiply(Quaternion const& _q) const {
	float scalar = s * _q.s - glm::dot(v, _q.v);
	glm::vec3 vector = (_q.v * s) + (v * _q.s) + glm::cross(v, _q.v);
	return Quaternion{ scalar, vector };
}

void Quaternion::operator*=(Quaternion const& _q) {
	(*this) = Multiply(_q);
}

float Quaternion::Dot(Quaternion const& _q) const {
	return (s * _q.s) + glm::dot(v, _q.v);
}

float Quaternion::Magnitude() const {
	float scalar = s * s;
	float imaginary = glm::dot(v, v);
	return sqrtf(scalar + imaginary);
}

void Quaternion::Normalize() {
	float magnitude = Magnitude();
	if (magnitude != 0.0f) {
		s /= magnitude;
		v /= magnitude;
	}
}

Quaternion Quaternion::Conjugate() const {
	float scalar = s;
	glm::vec3 vector = v * (-1.0f);
	return Quaternion{ scalar, vector };
}

Quaternion Quaternion::Inverse() const {
	float magnitude = Magnitude();
	return Conjugate() / (magnitude * magnitude);
}

glm::mat3 Quaternion::ToMat3() const {
	float m00 = 1 - (2 * ((v.y * v.y) + (v.z * v.z)));
	float m01 = 2 * ((v.x * v.y) - (s * v.z));
	float m02 = 2 * ((v.x * v.z) + (s * v.y));
	float m10 = 2 * ((v.x * v.y) + (s * v.z));
	float m11 = 1 - (2 * ((v.x * v.x) + (v.z * v.z)));
	float m12 = 2 * ((v.y * v.z) - (s * v.x));
	float m20 = 2 * ((v.x * v.z) - (s * v.y));
	float m21 = 2 * ((v.y * v.z) + (s * v.x));
	float m22 = 1 - (2 * ((v.x * v.x) + (v.y * v.y)));
	return glm::mat3{ m00, m01, m02,
						m10, m11, m12,
						m20, m21, m22 };
}

glm::vec3 Quaternion::operator*(glm::vec3 const& _r) const {
	return glm::vec3{ ((s * s) - (glm::dot(v, v)) * _r)
					+ (2 * (glm::dot(v, _r)))
					+ (2 * s * glm::cross(v, _r)) };
}

glm::vec3 Quaternion::Multiply(glm::vec3 const& _r) const {
	return glm::vec3{ ((s * s) - (glm::dot(v, v)) * _r)
					+ (2 * (glm::dot(v, _r)))
					+ (2 * s * glm::cross(v, _r)) };
}

glm::vec3 Quaternion::Rotate(glm::vec3 const& _r) const {
	return glm::vec3{ ((s * s) - (glm::dot(v, v)) * _r)
					+ (2 * (glm::dot(v, _r)))
					+ (2 * s * glm::cross(v, _r)) };
}