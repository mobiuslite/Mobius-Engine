#pragma once
#include "cComponent.h"
#include "cBowComponent.h"
#include "cParticleSystem.h"

class cTarget : public cComponent
{
public:
	cTarget(cBowComponent* otherBow, cEntityManager* manager, cParticleSystem* particleSystem);

	virtual ~cTarget() {};
	virtual void Update(float dt);
	virtual void Awake();

private:
	cTransform* transform;
	cBowComponent* bow;

	cEntityManager* entityManager;
	cParticleSystem* particleSystem;

	float riseSpeed;

	const float TARGET_RADIUS = 1.0f;
};