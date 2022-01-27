#include "cWorld.h"
#include <algorithm>
#include <glm/gtx/euler_angles.hpp>

#include <iostream>

cWorld::cWorld()
{
	this->particleList = new std::vector<cParticle*>();
	this->forceRegistry = new cForceRegistry();
}
cWorld::~cWorld()
{
	//Removes and deletes all particles
	for (std::vector<cParticle*>::iterator particleIt = this->particleList->begin(); particleIt != this->particleList->end(); particleIt = this->particleList->begin())
	{
		cParticle* curParticle = *particleIt;

		this->RemoveParticleFromWorld(curParticle);

		delete curParticle->entity;
		delete curParticle;
	}

	delete this->particleList;
	delete this->forceRegistry;
}

std::vector<cParticle*>* cWorld::GetParticleList()
{
	return this->particleList;
}
cForceRegistry* cWorld::GetForceRegistry()
{
	return this->forceRegistry;
}
cParticle* cWorld::CreateParticle(float mass, float radius, float damping, float timeOfDeath)
{
	return new cParticle(mass, radius, damping, timeOfDeath);
}

bool cWorld::AddParticleToWorld(cParticle* addParticle)
{
	if (addParticle == nullptr)
	{
		return false;
	}

	//Checks to see if the particle already exists.
	for (cParticle* particle : *particleList)
	{
		if (particle == addParticle)
		{
			return false;
		}
	}

	particleList->push_back(addParticle);
	return true;
}

bool cWorld::RemoveParticleFromWorld(cParticle* findParticle)
{
	if (findParticle == nullptr)
	{
		return false;
	}

	int particleIndex = 0;
	//Finds the particle
	for (cParticle* particle : *particleList)
	{
		if (particle == findParticle)
		{
			this->forceRegistry->Deregister(particle);
			particleList->erase(particleList->begin() + particleIndex);
			return true;
		}
		particleIndex++;
	}

	//Returns false if no particle was found
	return false;
}

void cWorld::EulerTimeStep(float deltaTime)
{
	//Apply forces
	this->forceRegistry->UpdateForces(deltaTime);

	//Integrate particles and delete old particles
	std::vector<cParticle*> deadParticles = IntegrateParticleList(deltaTime);
	this->DeleteParticles(&deadParticles);
}

std::vector<cParticle*> cWorld::IntegrateParticleList(float deltaTime)
{
	std::vector<cParticle*> deadParticles;

	//Loops through every particle and integrates it with the world.
	for (cParticle* particle : *particleList)
	{
		if (particle->Integrate(deltaTime))
		{
			//If integrate returns true, it's time to remove the particle.
			deadParticles.push_back(particle);
		}
	}

	return deadParticles;
}

void cWorld::DeleteParticles(std::vector<cParticle*>* deadParticles)
{
	if (deadParticles->size() > 0)
	{		
		for (size_t i = 0; i < deadParticles->size(); i++)
		{
			for (std::vector<cParticle*>::iterator partIt = this->particleList->begin(); partIt != this->particleList->end(); partIt)
			{
				cParticle* curParticle = *partIt;

				if (curParticle == deadParticles->at(i))
				{
					this->forceRegistry->Deregister(curParticle);
					this->particleList->erase(partIt);

					delete curParticle->entity;
					delete curParticle;

					break;
				}
				else
				{
					partIt++;
				}
			}
		}				
	}
	deadParticles->clear();
}