#include "cForceRegistry.h"

cForceRegistry::cForceRegistry()
{

}
cForceRegistry::~cForceRegistry()
{

}

void cForceRegistry::Register(cParticle* particle, iForceGenerator* forceGen)
{
	if (particle != nullptr && forceGen != nullptr)
	{
		this->registry.insert(std::pair<cParticle*, iForceGenerator*>(particle, forceGen));
	}
}

void cForceRegistry::Deregister(cParticle* particle)
{
	if (particle != nullptr)
	{
		this->registry.erase(particle);
	}
}

void cForceRegistry::UpdateForces(float deltaTime)
{
	for (std::multimap<cParticle*, iForceGenerator*>::iterator regIt = this->registry.begin(); regIt != this->registry.end(); regIt++)
	{
		regIt->second->Integrate(regIt->first, deltaTime);
	}
}