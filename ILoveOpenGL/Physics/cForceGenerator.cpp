#include "cForceGenerator.h"

cForceGenerator::cForceGenerator(glm::vec3 force)
{
	this->forceAcceleration = force;
}

cForceGenerator::~cForceGenerator()
{
}

void cForceGenerator::Integrate(cParticle* particle, float deltaTime)
{
	if (particle->Position().y >= 0)
	{
		float mass = particle->Mass();
		particle->ApplyForce(this->forceAcceleration * mass);
	}
}