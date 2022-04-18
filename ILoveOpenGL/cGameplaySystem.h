#pragma once
#include "cEntityManager.h"
#include "cParticleSystem.h"
#include "cBowComponent.h"

class cGameplaySystem
{
public:
	cGameplaySystem(cEntityManager* manager, cParticleSystem* particle, cBowComponent* bow);

	void Update(float dt);

private:
	cEntityManager* entityManager;
	cParticleSystem* particleSystem;
	cBowComponent* bowComp;

	const float balloonSpawnTime = 200.0f;
	float elapsedBalloonTime;
};