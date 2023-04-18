#include "stdafx.h"

#include "PhysicsManager.h"

#include "Cloth.h"

PhysicsManager::PhysicsManager():
	cloth{ nullptr }
{
}

void PhysicsManager::SetupCloth() {
	// setup cloth
	cloth = new Cloth{};
}

void PhysicsManager::SetupCloth(Cloth* _cloth) {
	cloth = _cloth;
}

void PhysicsManager::UpdateCloth(float dt) {
	cloth->Update(dt);
}
