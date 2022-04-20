#pragma once
#include "cTransform.h"

class cBowComponent;

class cProjectile : public cComponent
{
public:
	cProjectile(cBowComponent* parent, cTransform* meshTransform, glm::vec3 direction, float speed);

	virtual ~cProjectile() {};
	virtual void Update(float dt);

	cTransform GetProjTransform();

	void HitTarget();

private:

	bool hitTarget = false;

	float elapsedLifetime;
	const float lifetime = 60.0f;

	cBowComponent* parent;
	cTransform* transform;
	glm::vec3 velocity;

	const glm::vec3 GRAVITY = glm::vec3(0.0f, -11.0f, 0.0f);
};