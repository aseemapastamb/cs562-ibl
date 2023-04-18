#include "stdafx.h"

#include "VQS.h"

VQS::VQS() :
	v{ 0.0f },
	q{},
	s{ 0.0f } {

}

VQS::VQS(glm::vec3& _v, Quaternion& _q, float _s) :
	v{ _v },
	q{ _q },
	s{ _s } {

}

VQS::VQS(VQS const& _t) :
	v{ _t.v },
	q{ _t.q },
	s{ _t.s } {

}

VQS& VQS::operator=(VQS const& _t) {
	v = _t.v;
	q = _t.q;
	s = _t.s;
	return *this;
}

VQS VQS::operator+(VQS const& _t) const {
	glm::vec3 vector = v + _t.v;
	Quaternion quat = q + _t.q;
	float scalar = s + _t.s;
	return VQS{ vector, quat, scalar };
}

void VQS::operator+=(VQS const& _t) {
	v += _t.v;
	q += _t.q;
	s += _t.s;
}

VQS VQS::operator-(VQS const& _t) const {
	glm::vec3 vector = v - _t.v;
	Quaternion quat = q - _t.q;
	float scalar = s - _t.s;
	return VQS{ vector, quat, scalar };
}

void VQS::operator-=(VQS const& _t) {
	v -= _t.v;
	q -= _t.q;
	s -= _t.s;
}

VQS VQS::operator*(float const& _s) const {
	glm::vec3 vector = v * _s;
	Quaternion quat = q * _s;
	float scalar = s * _s;
	return VQS{ vector, quat, scalar };
}

void VQS::operator*=(float const& _s) {
	v *= _s;
	q *= _s;
	s *= _s;
}

// non-class function
// first operator is scalar, so swap
VQS operator*(float _s, VQS const& _t) {
	return _t * _s;
}

VQS VQS::operator*(VQS const& _t) const {
	glm::vec3 vector = Rotate(_t.v);
	Quaternion quat = q * _t.q;
	float scalar = s * _t.s;
	return VQS{ vector, quat, scalar };
}

VQS VQS::Multiply(VQS const& _t) const {
	glm::vec3 vector = Rotate(_t.v);
	Quaternion quat = q * _t.q;
	float scalar = s * _t.s;
	return VQS{ vector, quat, scalar };
}

void VQS::operator*=(VQS const& _t) {
	(*this) = Multiply(_t);
}

VQS VQS::operator/(float const& _s) const {
	glm::vec3 vector = v / _s;
	Quaternion quat = q / _s;
	float scalar = s / _s;
	return VQS{ vector, quat, scalar };
}

void VQS::operator/=(float const& _s) {
	v /= _s;
	q /= _s;
	s /= _s;
}

VQS VQS::Inverse() const {
	Quaternion quat = q.Inverse();
	glm::vec3 vector = quat.Rotate((-v) / s);
	float scalar = 1.0f / s;
	return VQS{ vector, quat, scalar };
}

glm::vec3 VQS::operator*(glm::vec3 const& _r) const {
	return (q.Rotate(s * _r) + v);
}

glm::vec3 VQS::Multiply(glm::vec3 const& _r) const {
	return (q.Rotate(s * _r) + v);
}

glm::vec3 VQS::Rotate(glm::vec3 const& _r) const {
	return (q.Rotate(s * _r) + v);
}
