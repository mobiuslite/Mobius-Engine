#pragma once
#include <vector>
#include "cParticle.h"
#include <glm/vec3.hpp>
#include "cEntityManager.h"

class cParticleSystem
{
public:

	cParticleSystem(cEntityManager* manager);

	void Update(float dt);
	void AddParticle(glm::vec3 pos, glm::vec3 velo, float lifeTime);

	const std::vector<cParticle*> GetParticles() { return vecParticles; }
private:
	std::vector<cParticle*> vecParticles;

	cEntityManager* manager;
};