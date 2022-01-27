#pragma once
#include<glm/vec3.hpp>
#include<glm/common.hpp>
#include <glm/exponential.hpp>
#include "../cEntity.h"

class cParticle
{
public:
	cParticle(float mass, float radius, float damping, float timeOfDeath);
	virtual ~cParticle();

	cParticle() = delete;
	cParticle(cParticle& other) = delete;
	cParticle& operator=(cParticle& other) = delete;

	void SetMesh(cEntity* mesh);

	void ApplyForce(const glm::vec3& force);
	void ClearAppliedForces();

	//Returns true if particle is dead.
	virtual bool Integrate(float deltaTime);

	float Mass();
	float InvsMass();

	glm::vec3 Position();
	void SetPosition(glm::vec3 pos);

	glm::vec3 Velocity();
	void SetVelocity(glm::vec3 velo);

	glm::vec3 Acceleration();

	float radius;
	cEntity* entity;

	float timeOfDeath;
	float age;

	bool removeFromWorld;
private:
	glm::vec3* position;
	glm::vec3 velocity;
	glm::vec3 acceleration;

	float inverseMass;

	glm::vec3 appliedForce;
	float damping;

	glm::vec3 startingScale;

};
