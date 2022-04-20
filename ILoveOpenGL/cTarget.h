#pragma once
#include "cComponent.h"
#include "cBowComponent.h"
#include "cParticleSystem.h"
#include "cGameplaySystem.h"

class cTarget : public cComponent
{
public:
	cTarget(cBowComponent* otherBow, cEntityManager* manager, cParticleSystem* particleSystem, cGameplaySystem* gameplaySystem, float minRiseSpeed, float maxRiseSpeed);

	virtual ~cTarget() {};
	virtual void Update(float dt);
	virtual void Awake();

	bool rise = true;

private:
	cTransform* transform;
	cBowComponent* bow;

	cGameplaySystem* parent;

	cEntityManager* entityManager;
	cParticleSystem* particleSystem;

	float riseSpeed;
	const float TARGET_RADIUS = 1.0f;
};