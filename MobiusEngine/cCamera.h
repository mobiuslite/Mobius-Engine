#pragma once
#include <glm/vec3.hpp>
#include <glm/vec2.hpp>

/// <summary>
/// Controls the camera eye, direction, and rotation.
/// </summary>
class cCamera
{
public:
	cCamera();

	void Translate(glm::vec3 translation);
	void SetEye(glm::vec3 pos);

	void SetDebugMode(bool state);

	//Sets the bounds of the player while in gameplay mode. Player must be between x.x, x.y, and z.x, z.y. Player will be forced onto y position
	void SetGameplayBounds(glm::vec2 x, glm::vec2 z, float y);

	void AddPitch(float pitch);
	void AddYaw(float yaw);

	//Updates the camera right, forward, and up directions. Call this when changing the pitch or yaw
	void Update();

	glm::vec3 GetEye();
	glm::vec3* GetEyePointer();

	glm::vec3 GetRight();
	glm::vec3 GetForward();
	glm::vec3 GetUp();


	float mouseSense = 0.1f;
private:

	bool debug;

	glm::vec3 cameraEye = glm::vec3(0);

	glm::vec3 cameraDir = glm::vec3(0.0f, 0.0f, 1.0f);
	glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
	glm::vec3 cameraRight = glm::vec3(0);

	float yaw = 90.0f;
	float pitch = 0.0f;	

	glm::vec2 xBounds;
	glm::vec2 zBounds;
	float yPos;
};