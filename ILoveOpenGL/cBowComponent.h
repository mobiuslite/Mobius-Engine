#pragma once
#include "cComponent.h"
#include "cProjectile.h"
#include "cEntityManager.h"

class cBowComponent : public cComponent
{
public:
	cBowComponent(cTransform* meshTransform, glm::vec3* cameraPos, cEntityManager* manager);

	virtual ~cBowComponent() {};
	virtual void Update(float dt);

	void FireProjectile(glm::vec3 pos, glm::vec3 direction, float aimingValue);

	std::vector<cProjectile*> GetFiredProjectiles();
	void CleanUpProjectile();

	float GetAimingValue();
	void PoppedBalloon();

	float GetAccuracy();

	bool aiming;
	const float fovChangeAmount = 25.0f;
private:

	std::vector<cProjectile*> projectilesFired;
	cEntityManager* entityManager;

	cTransform* transform;
	glm::vec3* cameraPos;
	glm::vec3 offset;

	float aimingValue = 0.0f;
	float aimingSpeed = 0.75f;	

	unsigned int projectilesFiredCount;
	unsigned int balloonsPopped;
};