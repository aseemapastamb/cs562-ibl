#pragma once

// fwd decl
class Cloth;

class PhysicsManager {
public:

public:
	PhysicsManager();
	~PhysicsManager() = default;

	void SetupCloth();
	void SetupCloth(Cloth* _cloth);
	void UpdateCloth(float dt);

	inline Cloth* GetCloth() const { return cloth; }

private:
	Cloth* cloth;

private:

};

extern PhysicsManager* p_physics_manager;