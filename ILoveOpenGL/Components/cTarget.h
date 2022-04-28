#pragma once
#include "cComponent.h"
#include "cBowComponent.h"
#include "../Systems/cParticleSystem.h"
#include "../Systems/cGameplaySystem.h"

//Component that allows the entity to act as a target
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