#pragma once
#include "cComponent.h"
#include "cTransform.h"

class cBowComponent : public cComponent
{
public:
	cBowComponent(cTransform* meshTransform, glm::vec3* cameraPos);

	virtual ~cBowComponent() {};
	virtual void Update(float dt);

private:

	cTransform* transform;
	glm::vec3* cameraPos;
	glm::vec3 offset;
};