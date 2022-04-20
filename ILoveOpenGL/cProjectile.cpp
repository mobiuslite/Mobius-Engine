#include "cProjectile.h"

cProjectile::cProjectile(cTransform* meshTransform, glm::vec3 direction, float speed)
{
	this->transform = meshTransform;
	this->velocity = glm::normalize(direction) * speed;

	this->isUpdatable = true;

	this->elapsedLifetime = 0.0f;
}

void cProjectile::Update(float dt)
{
	this->elapsedLifetime += dt;
	if (this->elapsedLifetime >= this->lifetime)
	{
		readyForCleanup = true;
	}

	this->transform->Translate(this->velocity * dt);
	
	if (this->transform->position.y <= 2.25f || this->hitTarget)
	{
		this->velocity = glm::vec3(0.0f);
	}
	else
	{
		this->velocity += this->GRAVITY * dt;
	}

	if (glm::length(velocity) > 0.0f)
	{
		glm::vec3 direction = glm::normalize(this->velocity);
		glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);

		glm::mat3 rotationMat = glm::lookAt(transform->position, transform->position + direction, up);

		//Add 90 degrees to fix model rotation
		transform->SetRotation(glm::quat_cast(rotationMat) * glm::quat(glm::vec3(0.0f, 90.0f, 0.0f)));
	}
}

cTransform cProjectile::GetProjTransform()
{
	return *this->transform;
}

void cProjectile::HitTarget()
{
	this->hitTarget = true;
}
