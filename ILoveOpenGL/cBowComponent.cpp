#include "cBowComponent.h"
#include "cMeshRenderer.h"
#include "cProjectile.h"
#include <iostream>

cBowComponent::cBowComponent(cTransform* meshTransform, glm::vec3* cameraPos, cEntityManager* manager)
{
	this->transform = meshTransform;
	this->cameraPos = cameraPos;

	this->isUpdatable = true;

	this->offset = glm::vec3(-.8f, -0.5f, 1.25f);

    this->aiming = false;

    this->entityManager = manager;

    projectilesFiredCount = 0;
    balloonsPopped = 0;
}

void cBowComponent::GameStart()
{
    for (cProjectile* proj : this->projectilesFired)
    {
        proj->GetEntity()->Delete();
    }
    projectilesFired.clear();

    this->balloonsPopped = 0;
    this->projectilesFiredCount = 0;
}

void cBowComponent::GameDone()
{
    std::cout << "Post game results!: " << std::endl << std::endl;
    std::cout << "\tAccuracy: " << ((this->balloonsPopped / (float)this->projectilesFiredCount) * 100.0f) << "%" << std::endl;
    std::cout << "\tBalloons popped: " << this->balloonsPopped << std::endl;
    std::cout << "\tArrows fired: " << this->projectilesFiredCount << std::endl << std::endl;
}

void cBowComponent::Update(float dt)
{
	transform->position = *cameraPos + offset;

    if (aiming && aimingValue < 1.0f)
    {
        aimingValue += aimingSpeed * dt;
    }

    for (size_t i = 0; i < this->projectilesFired.size();)
    {
        cProjectile* curProj = this->projectilesFired[i];

        if (curProj->IsReadyForCleanup())
        {
            curProj->GetEntity()->Delete();
            this->projectilesFired.erase(this->projectilesFired.begin() + i);
        }
        else
        {
            i++;
        }
    }
}

std::vector<cProjectile*> cBowComponent::GetFiredProjectiles()
{
    return this->projectilesFired;
}

void cBowComponent::CleanUpProjectile()
{
    for (cProjectile* proj : this->projectilesFired)
    {
        entityManager->DeleteEntity(proj->GetEntity());
        proj = nullptr;
    }

    this->projectilesFired.clear();
}

float cBowComponent::GetAimingValue()
{
    return this->aimingValue;
}

void cBowComponent::PoppedBalloon()
{
    this->balloonsPopped++;
}

float cBowComponent::GetAccuracy()
{
    return (this->balloonsPopped / ((float)this->projectilesFiredCount)) * 100.0f;
}

void cBowComponent::FireProjectile(glm::vec3 pos, glm::vec3 direction, float aimingValue)
{
    cEntity* newProj = this->entityManager->CreateEntity();
    newProj->name = "Projectile";
    newProj->isGameplayEntity = true;

    cMeshRenderer* mesh = newProj->AddComponent<cMeshRenderer>();
    mesh->meshName = "arrow.fbx";
    mesh->wholeObjectDiffuseRGBA = glm::vec4(1.0f, 0.1f, 0.1f, 1.0f);
    mesh->bUseWholeObjectDiffuseColour = true;

    cTransform* trans = newProj->GetComponent<cTransform>();
    trans->position = pos;
    trans->scale = glm::vec3(0.06f);

    cProjectile* proj = new cProjectile(trans, direction, 65.0f * aimingValue + 10.0f);

    newProj->AddComponent(proj);

    this->projectilesFired.push_back(proj);

    this->aiming = false;
    this->aimingValue = 0.0f;

    this->projectilesFiredCount++;
}