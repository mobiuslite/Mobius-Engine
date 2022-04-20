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

	bool rise = true;

private:
	cTransform* transform;
	cBowComponent* bow;

	cEntityManager* entityManager;
	cParticleSystem* particleSystem;

	float riseSpeed;
	float elapsedLifetime;

	const float TARGET_RADIUS = 1.0f;
	const float TARGET_LIFETIME = 15.0f;
};