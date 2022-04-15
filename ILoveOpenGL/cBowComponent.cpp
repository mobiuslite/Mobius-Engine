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
}


void cBowComponent::Update(float dt)
{
	transform->position = *cameraPos + offset;
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

void cBowComponent::FireProjectile(glm::vec3 pos, glm::vec3 direction, float aimingValue)
{
    cEntity* newProj = this->entityManager->CreateEntity();
    newProj->name = "Projectile";
    newProj->isGameplayEntity = true;

    cMeshRenderer* mesh = newProj->AddComponent<cMeshRenderer>();
    mesh->meshName = "ISO.ply";
    mesh->wholeObjectDiffuseRGBA = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);
    mesh->bUseWholeObjectDiffuseColour = true;

    cTransform* trans = newProj->GetComponent<cTransform>();
    trans->position = pos;
    trans->scale = glm::vec3(0.2f);

    cProjectile* proj = new cProjectile(trans, direction, 50.0f * aimingValue + 10.0f);

    newProj->AddComponent(proj);

    this->projectilesFired.push_back(proj);
}