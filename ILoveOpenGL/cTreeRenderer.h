#pragma once

#include "cComponent.h"
#include <glm/vec3.hpp>
#include <glm/gtx/quaternion.hpp>
#include <vector>
#include <glad/glad.h>
#include "cFBO/cFBO.h"

class cTreeRenderer : public cComponent
{
public:
	cTreeRenderer(unsigned int treeAmount, float offset);

	virtual ~cTreeRenderer() {};
	virtual void Update(float dt) {};

	unsigned int GetTreeCount();
	bool GetUniformSetupComplete();

	void SetupUniformLocations(unsigned int shaderId);

private:
	bool uniformSetupComplete = false;
	unsigned int treeAmount;
	float offset;

	std::vector<glm::vec4> translations;


};