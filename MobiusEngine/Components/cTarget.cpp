#include "cTarget.h"
#include "cMeshRenderer.h"
#include "../FMODSoundpanel/cSoundPanel.h"


cTarget::cTarget(cBowComponent* bow, cEntityManager* entityManager, cParticleSystem* particleSystem, cGameplaySystem* gameplaySystem, float minRiseSpeed, float maxRiseSpeed)
{
	this->bow = bow;
	this->isUpdatable = true;
	this->transform = nullptr;

	this->parent = gameplaySystem;

	this->entityManager = entityManager;
	this->particleSystem = particleSystem;

	if (maxRiseSpeed > 0.0f && minRiseSpeed > 0.0f)
		this->riseSpeed = rand() % (int)(maxRiseSpeed - minRiseSpeed) + minRiseSpeed;
	else
		this->riseSpeed = 0.0f;

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
					const float lifetime = 2.5f;

					float x = ((rand() % 21) / 10.0f) - 1.0f;
					float y	= ((rand() % 21) / 10.0f) - 1.0f;
					float z = ((rand() % 21) / 10.0f) - 1.0f;

					direction = glm::normalize(glm::vec3(x, y, z));

					particleSystem->AddParticle(transform->position, direction * speed, lifetime);
				}

				this->bow->PoppedBalloon();

				if(rise)
					this->parent->RemoveBalloon(this->GetEntity());
				this->GetEntity()->Delete();
				return;
			}
		}
	}
}