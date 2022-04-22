#include "cProjectile.h"
#include "cBowComponent.h"

cProjectile::cProjectile(cBowComponent* parent, cTransform* meshTransform, glm::vec3 direction, float speed)
{
	this->transform = meshTransform;
	this->velocity = glm::normalize(direction) * speed;
	this->parent = parent;

	this->isUpdatable = true;

	this->elapsedLifetime = 0.0f;
}

void cProjectile::Update(float dt)
{
	this->elapsedLifetime += dt;
	if (this->elapsedLifetime >= this->lifetime)
	{
		parent->RemoveProjectile(this->GetEntity());
		this->GetEntity()->Delete();
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
		//Rotate arrow so it is aligned with the bow when firing
		glm::vec3 dir = glm::normalize(velocity);

		float rotY = -1.f * atan2(dir.z, dir.x);		
		transform->SetRotation(glm::vec3(0.0f, rotY, 0.0f));
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
