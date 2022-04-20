#pragma once
#include "cEntityManager.h"
#include "cParticleSystem.h"
#include "cBowComponent.h"

class cGameplaySystem
{
public:
	cGameplaySystem(cEntityManager* manager, cParticleSystem* particle, cBowComponent* bow);

	void Update(float dt);
	bool playing = false;

private:
	cEntityManager* entityManager;
	cParticleSystem* particleSystem;
	cBowComponent* bowComp;

	const float balloonSpawnTime = 2.0f;
	const float gameTime = 30.0f;

	float elapsedBalloonTime;
	float elapsedGameTime;
};