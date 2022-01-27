#pragma once
#include "iForceGenerator.h"

class cForceGenerator : public iForceGenerator
{
public:
	cForceGenerator(glm::vec3 force);
	cForceGenerator() = delete;

	virtual ~cForceGenerator();

	virtual void Integrate(cParticle* particle, float deltaTime);

private:
	glm::vec3 forceAcceleration;
};