#pragma once
#include <glm/vec3.hpp>
#include "cComponent.h"
#include "cTransform.h"

class cParticle : public cComponent
{
public:
	cParticle(glm::vec3 velocity, float lifeTime);

	virtual void Awake();
	virtual void Update(float dt);

	const bool ReadyForCleanup() { return readyForCleanup; }

private:
	cTransform* transform;

	float lifeTime;
	float elapsedLifetime;

	bool readyForCleanup = false;

	glm::vec3 velocity;
	const glm::vec3 GRAVITY = glm::vec3(0.0f, -4.0f, 0.0f);
};