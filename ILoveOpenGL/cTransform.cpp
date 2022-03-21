#include "cTransform.h"

void cTransform::Update(float dt)
{

}
void cTransform::SetRotation(glm::quat q)
{
	this->rotation = q;
}
void cTransform::SetRotation(glm::vec3 v, bool degrees)
{
	glm::vec3 radians = v;
	if (degrees)
	{
		glm::vec3 radians = v;
		radians.x = glm::radians(radians.x);
		radians.y = glm::radians(radians.y);
		radians.z = glm::radians(radians.z);
	}
	this->rotation = glm::quat(radians);
}

glm::quat cTransform::GetQuatRotation()
{
	return this->rotation;
}
glm::vec3 cTransform::GetEulerRotation(bool degrees)
{
	glm::vec3 returnVec = glm::eulerAngles(this->rotation);
	if (degrees)
	{
		returnVec.x = glm::degrees(returnVec.x);
		returnVec.y = glm::degrees(returnVec.y);
		returnVec.z = glm::degrees(returnVec.z);
	}

	return returnVec;
}

void cTransform::Rotate(glm::quat q)
{
	this->rotation *= q;
}
void cTransform::Rotate(glm::vec3 v)
{
	SetRotation(this->GetEulerRotation() + v);
}