#include "cProjectile.h"

cProjectile::cProjectile(cTransform* meshTransform, glm::vec3 direction, float speed)
{
	this->transform = meshTransform;
	this->velocity = glm::normalize(direction) * speed;

	this->isUpdatable = true;
}

void cProjectile::Update(float dt)
{
	this->transform->Translate(this->velocity * dt);
	
	if (this->transform->position.y <= 2.25f)
	{
		this->velocity = glm::vec3(0.0f);
	}
	else
	{
		this->velocity += this->GRAVITY * dt;
	}
}
