#include "cBowComponent.h"

cBowComponent::cBowComponent(cTransform* meshTransform, glm::vec3* cameraPos)
{
	this->transform = meshTransform;
	this->cameraPos = cameraPos;

	this->isUpdatable = true;

	this->offset = glm::vec3(-.8f, -0.5f, 1.25f);
}


void cBowComponent::Update(float dt)
{
	transform->position = *cameraPos + offset;
}