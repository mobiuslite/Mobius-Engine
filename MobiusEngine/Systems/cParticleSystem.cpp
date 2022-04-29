#include "cParticleSystem.h"
#include "../Components/cMeshRenderer.h"

cParticleSystem::cParticleSystem(cEntityManager* manager)
{
	this->manager = manager;
}

void cParticleSystem::Update(float dt)
{
	for (int i = 0; i < this->vecParticles.size();)
	{
		cParticle* curPart = vecParticles[i];

		curPart->Update(dt);
		if (curPart->ReadyForCleanup())
		{
			this->vecParticles.erase(this->vecParticles.begin() + i);
			curPart->GetEntity()->Delete();
		}
		else
		{
			i++;
		}
	}
}

void cParticleSystem::AddParticle(glm::vec3 pos, glm::vec3 velo, float lifeTime)
{
	cEntity* newEntity = manager->CreateEntity();
	cTransform* newTransform = newEntity->GetComponent<cTransform>();
	cMeshRenderer* newMesh = newEntity->AddComponent<cMeshRenderer>();

	cParticle* newPart = new cParticle(velo, lifeTime);
	
	this->vecParticles.push_back(newPart);
	newEntity->AddComponent(newPart);

	newTransform->position = pos;
	newTransform->scale = glm::vec3(0.50f);

	newMesh->meshName = "ISO.ply";

	float rRandom = (rand() % 11) / 10.0f;
	float gRandom = (rand() % 11) / 10.0f;
	float bRandom = (rand() % 11) / 10.0f;

	newMesh->bUseWholeObjectDiffuseColour = true;
	newMesh->wholeObjectDiffuseRGBA = glm::vec4(rRandom, gRandom, bRandom, 1.0f);

	newEntity->name = "particle effect";
}