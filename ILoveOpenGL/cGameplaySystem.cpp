#include "cGameplaySystem.h"
#include "cMeshRenderer.h"
#include "cTransform.h"
#include "cTarget.h"
#include "FMODSoundpanel/cSoundPanel.h"

cGameplaySystem::cGameplaySystem(cEntityManager* manager, cParticleSystem* particle, cBowComponent* bow)
{
    this->entityManager = manager;
    this->particleSystem = particle;
    this->bowComp = bow;

    this->elapsedBalloonTime = 0.0f;
    this->elapsedGameTime = 0.0f;

    this->easy.balloonSpawnTime = 3.0f;
    this->easy.maxDistance = 20.0f;
    this->easy.maxRiseSpeed = 2.0f;
    this->easy.minRiseSpeed = 1.0f;

    this->normal.balloonSpawnTime = 2.0f;
    this->normal.maxDistance = 25.0f;
    this->normal.maxRiseSpeed = 3.0f;
    this->normal.minRiseSpeed = 1.0f;

    this->hard.balloonSpawnTime = 1.75f;
    this->hard.maxDistance = 35.0f;
    this->hard.maxRiseSpeed = 4.0f;
    this->hard.minRiseSpeed = 2.0f;

    this->curDifficulty = &normal;
}

void cGameplaySystem::Update(float dt)
{
    //Create new ballon
    if (playing)
    {

        elapsedBalloonTime += dt;
        elapsedGameTime += dt;
        if (elapsedBalloonTime >= curDifficulty->balloonSpawnTime)
        {
            elapsedBalloonTime = 0.0f;

            cEntity* newBalloon = entityManager->CreateEntity();
            newBalloon->name = "Target";
            newBalloon->isGameplayEntity = true;

            targets.push_back(newBalloon);

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

            float zRandom = (float)(rand() % (int)curDifficulty->maxDistance);
            zRandom -= 20.0f;

            newTransform->position.z = zRandom;

            newTransform->scale = glm::vec3(0.9f);

            cTarget* newTarget = new cTarget(bowComp, entityManager, particleSystem, this, curDifficulty->minRiseSpeed, curDifficulty->maxRiseSpeed);
            newBalloon->AddComponent(newTarget);
        }

        if (elapsedGameTime >= gameTime)
        {
            elapsedGameTime = 0.0f;
            elapsedBalloonTime = 0.0f;
            this->playing = false;

            for (cEntity* balloon : this->targets)
            {
                balloon->Delete();
            }
            this->targets.clear();

            this->bowComp->GameDone();
        }
    }
}

void cGameplaySystem::RemoveBalloon(cEntity* target)
{
    for (size_t i = 0; i < this->targets.size(); i++)
    {
        if (this->targets[i] == target)
        {
            this->targets.erase(this->targets.begin() + i);
            return;
        }
    }
}
