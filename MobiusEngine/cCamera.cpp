#include "cCamera.h"
#include <glm/trigonometric.hpp>
#include <glm/geometric.hpp>

cCamera::cCamera()
{
}

void cCamera::Update()
{
    glm::vec3 direction;
    direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    direction.y = sin(glm::radians(pitch));
    direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    cameraDir = glm::normalize(direction);

    cameraRight = glm::normalize(glm::cross(glm::vec3(0.0f, 1.0f, 0.0f), cameraDir));
    cameraUp = glm::cross(cameraDir, cameraRight);

    if (!debug)
    {
        cameraEye.y = this->yPos;

        //Limits the player's x coords
        if (cameraEye.x > xBounds.x)
        {
            cameraEye.x = xBounds.x;
        }
        if (cameraEye.x < xBounds.y)
        {
            cameraEye.x = xBounds.y;
        }

        //Limits the player's z coords
        if (cameraEye.z > zBounds.x)
        {
            cameraEye.z = zBounds.x;
        }
        if (cameraEye.z < zBounds.y)
        {
            cameraEye.z = zBounds.y;
        }
    }
}

void cCamera::Translate(glm::vec3 translation)
{
    this->cameraEye += translation;
}

void cCamera::SetEye(glm::vec3 pos)
{
    this->cameraEye = pos;
}

void cCamera::SetDebugMode(bool state)
{
    this->debug = state;
}

void cCamera::SetGameplayBounds(glm::vec2 x, glm::vec2 z, float y)
{
    this->xBounds = x;
    this->zBounds = z;
    this->yPos = y;
}

void cCamera::AddPitch(float p)
{
    this->pitch += p * mouseSense;

    if (pitch > 89.0f)
        pitch = 89.0f;
    if (pitch < -89.0f)
        pitch = -89.0f;
}
void cCamera::AddYaw(float y)
{
    this->yaw += y * mouseSense;
}

glm::vec3 cCamera::GetEye()
{
	return this->cameraEye;
}

glm::vec3* cCamera::GetEyePointer()
{
	return &this->cameraEye;
}

glm::vec3 cCamera::GetRight()
{
	return this->cameraRight;
}

glm::vec3 cCamera::GetForward()
{
	return this->cameraDir;
}

glm::vec3 cCamera::GetUp()
{
    return this->cameraUp;
}