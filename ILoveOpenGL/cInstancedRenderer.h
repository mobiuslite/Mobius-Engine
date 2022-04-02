#pragma once

#include "cComponent.h"
#include <glm/vec3.hpp>
#include <glm/gtx/quaternion.hpp>
#include <vector>
#include <glad/glad.h>
#include "cFBO/cFBO.h"
#include <map>
#include "cVAOManager.h"

class cInstancedRenderer : public cComponent
{
public:
	cInstancedRenderer(unsigned int amount, float offset);

	virtual ~cInstancedRenderer();
	virtual void Update(float dt) {};

	unsigned int GetCount();
	void SetupVertexArrayAttrib(sModelDrawInfo* drawInfo);

private:
	unsigned int amount;
	float offset;

	glm::vec4* translations;
	unsigned int instancedVBO_ID;
};