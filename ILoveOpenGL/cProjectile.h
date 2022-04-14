#pragma once
#include "cTransform.h"

class cProjectile : public cComponent
{
public:
	cProjectile(cTransform* meshTransform, glm::vec3 direction, float speed);

	virtual ~cProjectile() {};
	virtual void Update(float dt);

private:

	cTransform* transform;
	glm::vec3 velocity;

	const glm::vec3 GRAVITY = glm::vec3(0.0f, -11.0f, 0.0f);
};