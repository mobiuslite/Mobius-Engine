#include "cTransform.h"

void cTransform::Update()
{

}
void cTransform::SetRotation(glm::quat q)
{
	this->rotation = q;
}
void cTransform::SetRotation(glm::vec3 v)
{
	this->rotation = glm::quat(v);
}

glm::quat cTransform::GetQuatRotation()
{
	return this->rotation;
}
glm::vec3 cTransform::GetEulerRotation()
{
	return glm::eulerAngles(this->rotation);
}

void cTransform::Rotate(glm::quat q)
{
	this->rotation *= q;
}
void cTransform::Rotate(glm::vec3 v)
{
	SetRotation(this->GetEulerRotation() + v);
}