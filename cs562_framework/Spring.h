#pragma once

// fwd decl
class PointMass;

class Spring {
public:
	// 2 point masses on each end
	PointMass* pm_a;
	PointMass* pm_b;

	// length of spring at rest
	float rest_length;

	// spring coefficient
	float ks;

public:
	Spring(PointMass* _pm_a, PointMass* _pm_b);
	~Spring();
};