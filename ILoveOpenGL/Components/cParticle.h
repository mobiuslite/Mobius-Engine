#pragma once
#include <glm/vec3.hpp>
#include "cComponent.h"
#include "cTransform.h"

//Paraticle component that will make the entity act like a particle
class cParticle : public cComponent
{
public:
	cParticle(glm::vec3 velocity, float lifeTime, float damping = .5f);

	virtual void Awake();
	virtual void Update(float dt);

	const bool ReadyForCleanup() { return readyForCleanup; }

private:
	cTransform* transform;

	float lifeTime;
	float elapsedLifetime;

	float damping;

	bool readyForCleanup = false;

	glm::vec3 velocity;
	const glm::vec3 GRAVITY = glm::vec3(0.0f, -2.5f, 0.0f);
};