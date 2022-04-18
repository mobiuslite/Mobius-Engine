#include "cGameplaySystem.h"
#include "cMeshRenderer.h"
#include "cTransform.h"
#include "cTarget.h"

cGameplaySystem::cGameplaySystem(cEntityManager* manager, cParticleSystem* particle, cBowComponent* bow)
{
    this->entityManager = manager;
    this->particleSystem = particle;
    this->bowComp = bow;

    this->elapsedBalloonTime = 0.0f;
}

void cGameplaySystem::Update(float dt)
{
    //Create new ballon
    elapsedBalloonTime += dt;
    if (elapsedBalloonTime >= balloonSpawnTime)
    {
        elapsedBalloonTime = 0.0f;

        cEntity* newBalloon = entityManager->CreateEntity();
        newBalloon->name = "Target";
        newBalloon->isGameplayEntity = true;

        cMeshRenderer* newMesh = newBalloon->AddComponent<cMeshRenderer>();
        newMesh->meshName = "ISO.ply";
        newMesh->bUseWholeObjectDiffuseColour = true;
        newMesh->roughness = 0.4f;

        float rRandom = (rand() % 11) / 10.0f;
        float gRandom = (rand() % 11) / 10.0f;
        float bRandom = (rand() % 11) / 10.0f;

        newMesh->wholeObjectDiffuseRGBA = glm::vec4(rRandom, gRandom, bRandom, 1.0f);

        cTransform* newTransform = newBalloon->GetComponent<cTransform>();
        newTransform->position = glm::vec3(-19.0f, 4.1f, 8.4f);

        float xRandom = ((rand() % 21) - 10.0f);
        newTransform->position.x += xRandom;

        float zRandom = (float)(rand() % 25);
        zRandom -= 20.0f;

        newTransform->position.z = zRandom;

        newTransform->scale = glm::vec3(0.9f);

        cTarget* newTarget = new cTarget(bowComp, entityManager, particleSystem);
        newBalloon->AddComponent(newTarget);
    }
}