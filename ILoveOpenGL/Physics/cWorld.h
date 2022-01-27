#pragma once

#include "cParticle.h"
#include <vector>
#include "cForceRegistry.h"

class cWorld
{
public:
	cWorld();
	virtual ~cWorld();

	std::vector<cParticle*>* GetParticleList();
	cForceRegistry* GetForceRegistry();
	cParticle* CreateParticle(float mass, float radius, float damping, float timeOfDeath);

	bool AddParticleToWorld(cParticle* particle);
	bool RemoveParticleFromWorld(cParticle* particle);

	//Does an euler time step on the particle
	void EulerTimeStep(float deltaTime);

private:
	//Intergrates each particle in the world, and returns a list of dead particles if there are any
	std::vector<cParticle*> IntegrateParticleList(float deltaTime);
	void DeleteParticles(std::vector<cParticle*>* deadParticles);

	std::vector<cParticle*>* particleList;
	cForceRegistry* forceRegistry;
};
