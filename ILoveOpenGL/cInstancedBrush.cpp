#include "cInstancedBrush.h"

cInstancedBrush::cInstancedBrush()
{
	this->renderer = nullptr;

	this->lastPosAdded = glm::vec3(-100.0f, -100.0f, -100.0f);
}
cInstancedBrush::~cInstancedBrush()
{
}

void cInstancedBrush::AddOffset(glm::vec3 pos)
{
	if (this->renderer != nullptr)
	{
		if (glm::distance(pos, this->lastPosAdded) >= this->offset)
		{
			this->renderer->AddOffset(pos);
			this->lastPosAdded = pos;
		}
	}
}
void cInstancedBrush::ChangeRenderer(cInstancedRenderer* renderer)
{
	this->renderer = renderer;
}