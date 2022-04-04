#include "cInstancedRenderer.h"
#include "GLCommon.h"


cInstancedRenderer::cInstancedRenderer(unsigned int amount, float offset, float randomAmount)
{
	this->amount = amount;
	this->offset = offset;
    this->randomStrength = randomAmount;

    int eachAmount = (int)glm::sqrt(amount);
    int totalAmount = 0;

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
                int randomXOffset = (rand() % 200) * randomAmount;
                int randomZOffset = (rand() % 200) * randomAmount;

                translations.push_back( glm::vec4(x * offset + (randomXOffset / 100.0f), 0.0f, y * offset + (randomZOffset / 100.0f), 1.0f));
                totalAmount++;
            }
        }
    }

    glGenBuffers(1, &this->instancedVBO_ID);
}

void cInstancedRenderer::SetupVertexArrayAttrib(sModelDrawInfo* drawInfo)
{
    glBindVertexArray(drawInfo->VAO_ID);
    glBindBuffer(GL_ARRAY_BUFFER, this->instancedVBO_ID);

    glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec4) * this->translations.size(), (GLvoid*)&this->translations[0], GL_STATIC_DRAW);

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
}

unsigned int cInstancedRenderer::GetCount()
{
    return this->amount;
}

float cInstancedRenderer::GetOffset()
{
    return this->offset;
}
float cInstancedRenderer::GetRandomStrength()
{
    return this->randomStrength;
}