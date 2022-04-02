#include "cInstancedRenderer.h"
#include "GLCommon.h"


cInstancedRenderer::cInstancedRenderer(unsigned int amount, float offset)
{
	this->amount = amount;
	this->offset = offset;

    int eachAmount = (int)glm::sqrt(amount);
    int totalAmount = 0;

    translations = new glm::vec4[amount];

    for (int y = -eachAmount; totalAmount < amount; y += 2)
    {
        for (int x = -eachAmount; x < eachAmount; x += 2)
        {
            if (totalAmount >= this->amount)
            {
                break;
            }
            else
            {
                int randomXOffset = rand() % 200;
                int randomZOffset = rand() % 200;

                translations[totalAmount] = glm::vec4(x * offset + (randomXOffset / 100.0f), 0.0f, y * offset + (randomZOffset / 100.0f), 1.0f);
                totalAmount++;
            }
        }
    }
}

void cInstancedRenderer::SetupVertexArrayAttrib(sModelDrawInfo* drawInfo)
{
    glBindVertexArray(drawInfo->VAO_ID);

    glGenBuffers(1, &this->instancedVBO_ID);
    glBindBuffer(GL_ARRAY_BUFFER, this->instancedVBO_ID);

    glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec4) * amount, (GLvoid*)this->translations, GL_STATIC_DRAW);

    GLint vOffset_location = 6;
    glEnableVertexAttribArray(vOffset_location);
    glVertexAttribPointer(vOffset_location, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glVertexAttribDivisor(vOffset_location, 1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    glDisableVertexAttribArray(vOffset_location);
}

cInstancedRenderer::~cInstancedRenderer()
{
    delete[] this->translations;
}

unsigned int cInstancedRenderer::GetCount()
{
    return this->amount;
}