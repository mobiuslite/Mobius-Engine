#include "cEmitter.h"
#include "cEntity.h"
#include "cTransform.h"
#include "cMeshRenderer.h"

cEmitter::cEmitter(float time, glm::vec3 pos)
{
	this->time = time;
	this->position = pos;
}

void cEmitter::Integrate(float deltaTime, cWorld* world, iForceGenerator* force)
{
	this->elapsedTime += deltaTime;
	if (this->elapsedTime >= this->time)
	{
		this->elapsedTime = 0.0f;

		//Create particle

		//Mesh creation.
		cMeshRenderer* projectileMesh = new cMeshRenderer();

		projectileMesh->bIsImposter = true;
		projectileMesh->meshName = "plane.ply";

		//projectileMesh->wholeObjectDiffuseRGBA = glm::vec4(0.25f, 0.25f, 0.25f, 0.5f);
		projectileMesh->textures[0].ratio = 1.0f;
		projectileMesh->textures[0].name = "smoke.bmp";

		projectileMesh->friendlyName = "emitter particle";
		//projectileMesh->bDontLight = true;

		cParticle* particle = world->CreateParticle(1.0f, 0.25f, 0.25f, 5.0f);

		cEntity* newEntity = new cEntity();
		cTransform* newTransform = newEntity->GetComponent<cTransform>();

		glm::vec3 euler = glm::vec3(glm::radians(90.0f), glm::radians((float)(rand() % 360)), 0.0f);

		newTransform->SetRotation(euler);
		newTransform->scale = glm::vec3(0.33f);

		newEntity->AddComponent<cMeshRenderer>(projectileMesh);

		particle->SetMesh(newEntity);

		float randomX = (rand() % 200 + 100) / 1000.0f;
		randomX = (rand() % 2 == 1) ? randomX : randomX * -1.0f;

		float randomY = (rand() % 200 + 100) / 1000.0f;
		randomY = (rand() % 2 == 1) ? randomY : randomY * -1.0f;

		float randomZ = (rand() % 200 + 100) / 1000.0f;
		randomZ = (rand() % 2 == 1) ? randomZ : randomZ * -1.0f;

		particle->SetPosition(position + glm::vec3(randomX, randomY, randomZ));

		world->AddParticleToWorld(particle);
		world->GetForceRegistry()->Register(particle, force);
	}
}