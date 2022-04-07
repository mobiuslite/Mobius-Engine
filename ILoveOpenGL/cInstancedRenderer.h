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
	cInstancedRenderer(unsigned int amount, float offset, float randomAmount = 0.0f);

	virtual ~cInstancedRenderer();
	virtual void Update(float dt) {};

	unsigned int GetCount();

	float GetOffset();
	void AddOffset(glm::vec3 pos);

	float GetRandomStrength();

	void SetupVertexArrayAttrib(sModelDrawInfo* drawInfo);

private:
	float offset;
	float randomStrength;

	std::vector<glm::vec4> translations;
	unsigned int instancedVBO_ID;
};