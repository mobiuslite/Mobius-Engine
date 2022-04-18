#include "cParticle.h"
#include "cEntity.h"

cParticle::cParticle(glm::vec3 velocity, float lifeTime)
{
	this->velocity = velocity;
	this->isUpdatable = true;

	this->transform = nullptr;

	this->elapsedLifetime = 0.0f;
	this->lifeTime = lifeTime;
}

void cParticle::Awake()
{
	this->transform = this->GetEntity()->GetComponent<cTransform>();
}

void cParticle::Update(float dt)
{
	transform->position += this->velocity * dt;
	this->velocity += this->GRAVITY * dt;

	if (transform->scale.x > 0.0f)
	{
		transform->scale -= (1.0f / this->lifeTime) * dt;
	}
	else
	{
		transform->scale = glm::vec3(0.0f);
	}

	elapsedLifetime += dt;

	if (elapsedLifetime >= this->lifeTime)
	{
		readyForCleanup = true;
	}
}