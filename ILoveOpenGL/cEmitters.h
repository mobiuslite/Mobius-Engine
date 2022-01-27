#pragma once
#include "cEmitter.h"
#include <vector>

class cEmitters
{
public:
	cEmitters() = default;
	~cEmitters() = default;

	void AddEmitter(cEmitter* emit)
	{
		this->emitters.push_back(emit);
	}
	void SetWorld(cWorld* world)
	{
		this->world = world;
	}
	void SetForce(iForceGenerator* force)
	{
		this->force = force;
	}
	void Integrate(float deltaTime)
	{
		for (cEmitter* emit : this->emitters)
			emit->Integrate(deltaTime, this->world, this->force);
	}

private:

	cWorld* world = nullptr;
	iForceGenerator* force;
	std::vector<cEmitter*> emitters;

};