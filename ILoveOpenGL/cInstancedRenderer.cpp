#include "cInstancedRenderer.h"
#include "GLCommon.h"
#include <iostream>


cInstancedRenderer::cInstancedRenderer(unsigned int amount, float offset, float randomAmount)
{
	this->offset = offset;
    this->randomStrength = randomAmount;

    int eachAmount = (int)glm::sqrt(amount);
    int totalAmount = 0;

    for (int y = -eachAmount; totalAmount < amount; y += 2)
    {
        for (int x = -eachAmount; x < eachAmount; x += 2)
        {
            if (totalAmount >= amount)
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

    //glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec4) * this->translations.size(), (GLvoid*)&this->translations[0], GL_STATIC_DRAW);

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
    //GLuint bufferToDelete[1];
    //bufferToDelete[0] = this->instancedVBO_ID;
    //
    //glDeleteBuffers(1, bufferToDelete);
    //
    //GLenum err;
    //while ((err = glGetError()) != GL_NO_ERROR)
    //{
    //    std::cout << "WARNING: OpenGL errors found!: " << err << std::endl;
    //}
}

unsigned int cInstancedRenderer::GetCount()
{
    return (unsigned int)this->translations.size();
}

float cInstancedRenderer::GetOffset()
{
    return this->offset;
}
void cInstancedRenderer::AddOffset(glm::vec3 pos)
{
    glm::vec4 newPos = glm::vec4(pos.x, pos.y, pos.z, 1.0f);

    this->translations.push_back(newPos);

    glBindBuffer(GL_ARRAY_BUFFER, this->instancedVBO_ID);
    glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec4) * this->translations.size(), (GLvoid*)&this->translations[0], GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

}

float cInstancedRenderer::GetRandomStrength()
{
    return this->randomStrength;
}