#pragma once

#include "cComponent.h"
#include <glm/vec3.hpp>
#include <glm/gtx/quaternion.hpp>
#include <vector>
#include <glad/glad.h>
#include "../FBOs/cFBO/cFBO.h"
#include <map>
#include "../Managers/cVAOManager.h"

//Allows the entity to be instanced rendered (rendered multiple times with one draw call)
class cInstancedRenderer : public cComponent
{
public:
	cInstancedRenderer(unsigned int amount, float offset, std::string fileName, float randomAmount = 0.0f);

	virtual ~cInstancedRenderer();
	virtual void Update(float dt) {};

	unsigned int GetCount();

	float GetOffset();
	void AddOffset(glm::vec3 pos);
	void SaveOffsets();

	float GetRandomStrength();

	const bool UseFile() { return useFile; }

	void SetupVertexArrayAttrib(sModelDrawInfo* drawInfo);

	std::string fileName;
private:
	float offset;
	float randomStrength;

	bool useFile;

	std::vector<glm::vec4> translations;
	unsigned int instancedVBO_ID;
};