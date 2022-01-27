#include "cParticle.h"
#include "../cTransform.h"

cParticle::cParticle(float mass, float radius, float damping, float timeOfDeath)
{
	this->inverseMass = (mass <= 0.f) ? 0.f : 1.f / mass;

	this->position = nullptr;
	this->velocity = glm::vec3(0.0f);
	this->acceleration = glm::vec3(0.0f);
	this->radius = radius;
	this->damping = damping;
	this->timeOfDeath = timeOfDeath;

	this->removeFromWorld = false;

	this->entity = nullptr;
	this->age = 0.0f;

	this->appliedForce = glm::vec3(0.0f);

	this->startingScale = glm::vec3(-1.0f);
}
cParticle::~cParticle()
{

}

void cParticle::SetMesh(cEntity* mesh)
{
	this->entity = mesh;

	//When the particle is moved, it actually moves the mesh.
	cTransform* trans = mesh->GetComponent<cTransform>();

	this->position = &trans->position;

	this->startingScale = glm::vec3(trans->scale);
}

float cParticle::Mass()
{
	return (this->inverseMass == 0.f) ? inverseMass : 1.0f / inverseMass;
}
float cParticle::InvsMass()
{
	return this->inverseMass;
}

glm::vec3 cParticle::Position()
{
	return *this->position;
}
void cParticle::SetPosition(glm::vec3 pos)
{
	*this->position = pos;
}

glm::vec3 cParticle::Velocity()
{
	return this->velocity;
}
void cParticle::SetVelocity(glm::vec3 velo)
{
	this->velocity = velo;
}

glm::vec3 cParticle::Acceleration()
{
	return this->acceleration;
}

void cParticle::ApplyForce(const glm::vec3& force)
{
	appliedForce += force;
}
void cParticle::ClearAppliedForces()
{
	appliedForce = glm::vec3(0.f, 0.f, 0.f);
}

bool cParticle::Integrate(float deltaTime)
{
	if (this->removeFromWorld)
	{
		return true;
	}

	//If the mass is infinitely big, don't move it
	if (inverseMass == 0.f)
		return false;
	

	*position += velocity * deltaTime;
	velocity += (acceleration + appliedForce * inverseMass) * deltaTime;

	if (velocity.y > 1.0f)
		velocity.y = 1.0f;

	// apply damping
	velocity *= glm::pow(1/damping, deltaTime);

	//If time of death is positive, it will use a lifetime.
	if (this->timeOfDeath > 0.f)
	{
		this->age += deltaTime;

		if (glm::length(this->startingScale) != -1.0f && this->timeOfDeath - this->age < glm::length(this->startingScale))
		{
			this->entity->GetComponent<cTransform>()->scale = glm::vec3(this->timeOfDeath - this->age);
		}

		if (this->age > this->timeOfDeath)
		{
			return true;
		}
	}

	// clear applied forces
	ClearAppliedForces();

	return false;
}
