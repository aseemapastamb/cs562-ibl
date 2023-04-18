#include "stdafx.h"

#include "Spring.h"

#include "PointMass.h"

Spring::Spring(PointMass* _pm_a, PointMass* _pm_b):
	pm_a{ _pm_a },
	pm_b{ _pm_b },
	rest_length{ glm::distance(pm_a->position, pm_b->position) },
	ks{ 80.0f }
{
}

Spring::~Spring() {
	pm_a = nullptr;
	pm_b = nullptr;
}
