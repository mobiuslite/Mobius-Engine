#include "cBowComponent.h"
#include "cMeshRenderer.h"
#include "cProjectile.h"

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


void cBowComponent::Update(float dt)
{
	transform->position = *cameraPos + offset;

    if (aiming && aimingValue < 1.0f)
    {
        aimingValue += aimingSpeed * dt;
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
    mesh->wholeObjectDiffuseRGBA = glm::vec4(0.85f, 0.85f, 0.85f, 1.0f);
    mesh->bUseWholeObjectDiffuseColour = true;

    cTransform* trans = newProj->GetComponent<cTransform>();
    trans->position = pos;
    trans->scale = glm::vec3(0.02f);

    cProjectile* proj = new cProjectile(trans, direction, 50.0f * aimingValue + 10.0f);

    newProj->AddComponent(proj);

    this->projectilesFired.push_back(proj);

    this->aiming = false;
    this->aimingValue = 0.0f;

    this->projectilesFiredCount++;
}