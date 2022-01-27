#pragma once
#include "cParticle.h"
#include "iForceGenerator.h"
#include <map>

class cForceRegistry
{
public:
	cForceRegistry();
	~cForceRegistry();

	void Register(cParticle* particle, iForceGenerator* forceGen);
	void Deregister(cParticle* particle);

	void UpdateForces(float deltaTime);

private:
	std::multimap<cParticle*, iForceGenerator*> registry;
};