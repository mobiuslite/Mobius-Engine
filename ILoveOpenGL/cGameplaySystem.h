#pragma once
#include "cEntityManager.h"
#include "cParticleSystem.h"
#include "cBowComponent.h"

class cGameplaySystem
{
public:
	cGameplaySystem(cEntityManager* manager, cParticleSystem* particle, cBowComponent* bow);

	void Update(float dt);
	void SetNormalDifficult() { if(!this->playing) this->curDifficulty = &normal; }
	void SetEasyDifficult() { if (!this->playing) this->curDifficulty = &easy; }
	void SetHardDifficult() { if (!this->playing) this->curDifficulty = &hard; }

	void RemoveBalloon(cEntity* target);

	bool playing = false;

private:

	struct sDifficultySettings
	{
		float maxDistance;
		float balloonSpawnTime;

		float minRiseSpeed;
		float maxRiseSpeed;
	};

	sDifficultySettings* curDifficulty;
	sDifficultySettings hard;
	sDifficultySettings normal;
	sDifficultySettings easy;

	std::vector<cEntity*> targets;

	cEntityManager* entityManager;
	cParticleSystem* particleSystem;
	cBowComponent* bowComp;

	const float gameTime = 30.0f;

	unsigned int balloonsSpawned;

	float elapsedBalloonTime;
	float elapsedGameTime;
};