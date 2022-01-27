#pragma once
#include <glm/vec3.hpp>
#include "Physics/cWorld.h"

class cEmitter
{
public:
	cEmitter(float time, glm::vec3 position);
	~cEmitter() = default;

	void Integrate(float deltaTime, cWorld* world, iForceGenerator* force);

private:

	glm::vec3 position;

	float elapsedTime = 0.0f;
	float time;

};