#include "cTarget.h"
#include "cMeshRenderer.h"
#include "FMODSoundpanel/cSoundPanel.h"
#include <iostream>


cTarget::cTarget(cBowComponent* bow, cEntityManager* entityManager, cParticleSystem* particleSystem)
{
	this->bow = bow;
	this->isUpdatable = true;
	this->transform = nullptr;

	this->entityManager = entityManager;
	this->particleSystem = particleSystem;

	this->riseSpeed = ((float)(rand() % 31) / 10.0f) + 1.0f;
}

void cTarget::Awake()
{
	this->transform = this->GetEntity()->GetComponent<cTransform>();
}

void cTarget::Update(float dt)
{
	if (this->transform != nullptr)
	{
		if (rise)
		{
			this->elapsedLifetime += dt;
			if (this->elapsedLifetime >= this->TARGET_LIFETIME)
			{
				this->GetEntity()->Delete();
			}

			transform->position += glm::vec3(0.0f, riseSpeed, 0.0f) * dt;
		}

		std::vector<cProjectile*> bowProj = bow->GetFiredProjectiles();

		for (cProjectile* proj : bowProj)
		{
			//Projectile is in target
			if (glm::distance(proj->GetProjTransform().position, transform->position) <= this->TARGET_RADIUS)
			{
				cSoundPanel::GetInstance()->PlaySound("pop.mp3");

				//Load particles

				for (int i = 0; i < 10; i++)
				{
					glm::vec3 direction;
					const float speed = 3.5f;
					const float lifetime = 1.5f;

					float x = ((rand() % 21) / 10.0f) - 1.0f;
					float y	= ((rand() % 21) / 10.0f) - 1.0f;
					float z = ((rand() % 21) / 10.0f) - 1.0f;

					direction = glm::normalize(glm::vec3(x, y, z));

					particleSystem->AddParticle(transform->position, direction * speed, lifetime);
				}

				if (rise)
				{
					this->bow->PoppedBalloon();

					std::cout << "Accuracy: %" << this->bow->GetAccuracy() << std::endl;
				}

				this->GetEntity()->Delete();
				return;

			}
		}
	}
}