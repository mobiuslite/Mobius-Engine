#pragma once

#include "cComponent.h"
#include <glm/vec3.hpp>
#include <glm/gtx/quaternion.hpp>
#include <vector>

//Holds transform information for an entity (position, scale, rotation)
class cTransform : public cComponent
{
public:
	cTransform(glm::vec3 position, glm::quat rotation, glm::vec3 scale) : position(position), rotation(rotation), scale(scale) {}
	cTransform(glm::vec3 position, glm::vec3 rotation, glm::vec3 scale) : position(position), rotation(glm::quat(rotation)), scale(scale) {}
	cTransform() : position(glm::vec3(0.f)), rotation(glm::vec3(0.0f)), scale(1.0f){}

	void SetRotation(glm::quat q);
	void SetRotation(glm::vec3 v, bool degrees = false);

	glm::quat GetQuatRotation();
	glm::vec3 GetEulerRotation(bool degrees = false);

	void Rotate(glm::quat q);
	void Rotate(glm::vec3 v);

	void Translate(glm::vec3 t);

	virtual void Update(float dt);

	glm::vec3 position;
	glm::vec3 scale;

private:
	glm::quat rotation;
};