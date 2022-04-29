#include "cTextureViewer.h"

void cTextureViewer::SetupTextureIds(cFBO* fbo)
{
	this->text1Id = fbo->vertexMatColour_1_ID;
	this->text2Id = fbo->vertexMatColour_1_ID;
	this->text3Id = fbo->vertexMatColour_1_ID;
	this->text4Id = fbo->vertexMatColour_1_ID;
	this->text5Id = fbo->vertexMatColour_1_ID;

	this->curTextureId = this->text1Id;
}

cTextureViewer::cTextureViewer()
{
	this->isUpdatable = true;
	this->curTextureId = 0;

	this->switchTime += rand() % 4;
}

GLint cTextureViewer::GetCurrentTextId()
{
	return this->curTextureId;
}

void cTextureViewer::Update(float dt)
{
	this->elapsedTime += dt;

	if (this->elapsedTime >= this->switchTime)
	{
		curTextIndex++;
		if (curTextIndex > 4)
		{
			curTextIndex = 0;
		}

		switch (curTextIndex)
		{
		case 0:
			this->curTextureId = this->text1Id;
			break;
		case 1:
			this->curTextureId = this->text2Id;
			break;
		case 2:
			this->curTextureId = this->text3Id;
			break;
		case 3:
			this->curTextureId = this->text4Id;
			break;
		case 4:
			this->curTextureId = this->text5Id;
			break;
		}
	}
}