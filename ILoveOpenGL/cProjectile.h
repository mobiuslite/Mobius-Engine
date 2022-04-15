#pragma once
#include "cTransform.h"

class cProjectile : public cComponent
{
public:
	cProjectile(cTransform* meshTransform, glm::vec3 direction, float speed);

	virtual ~cProjectile() {};
	virtual void Update(float dt);

	cTransform GetProjTransform();

	void HitTarget();

	void CleanUp()
	{
		cleanUp = true;
	}

	bool IsReadyForCleanUp()
	{
		return cleanUp;
	}

private:

	bool cleanUp = false;
	bool hitTarget = false;

	cTransform* transform;
	glm::vec3 velocity;

	const glm::vec3 GRAVITY = glm::vec3(0.0f, -11.0f, 0.0f);
};