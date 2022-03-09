#pragma once
#include "cComponent.h"
#include <glm/vec3.hpp>
#include <glm/gtx/quaternion.hpp>
#include <vector>
#include <glad/glad.h>
#include "cFBO/cFBO.h"

class cTextureViewer : public cComponent
{
public:
	cTextureViewer();

	virtual ~cTextureViewer() {};
	virtual void Update(float dt);

	void SetupTextureIds(cFBO* fbo);

	GLint GetCurrentTextId();
	

private:
	
	GLint curTextureId;

	float switchTime = 3.0f;
	float elapsedTime = 0.0f;

	int curTextIndex = 0;
	GLint text1Id = 0;
	GLint text2Id = 0;
	GLint text3Id = 0;
	GLint text4Id = 0;
	GLint text5Id = 0;

};