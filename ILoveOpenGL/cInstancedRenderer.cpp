#include "cInstancedRenderer.h"
#include "GLCommon.h"
#include <iostream>
#include <fstream>


cInstancedRenderer::cInstancedRenderer(unsigned int amount, float offset, std::string fileName, float randomAmount)
{
	this->offset = offset;
    this->randomStrength = randomAmount;
    this->fileName = fileName;

    //If file exists
    std::ifstream inputFile(fileName);
    if (inputFile.is_open())
    {
        useFile = true;

        while (!inputFile.eof())
        {
            std::string xOffset;
            std::string yOffset;
            std::string zOffset;
    
            std::getline(inputFile, xOffset, ',');
            std::getline(inputFile, yOffset, ',');
            std::getline(inputFile, zOffset);
    
            glm::vec4 newTranslation = glm::vec4(std::stof(xOffset), std::stof(yOffset), std::stof(zOffset), 1.0f);
            translations.push_back(newTranslation);
        }
        inputFile.close();
    }

    //If file doesn't exist
    else
    {
        useFile = false;
        int fixedAmount = (int)sqrt(amount);

        for (int y = -fixedAmount; y < fixedAmount; y += 2)
        {
            for (int x = -fixedAmount; x < fixedAmount; x += 2)
            {
                int randomXOffset = (rand() % 200) * randomAmount;
                int randomZOffset = (rand() % 200) * randomAmount;

                translations.push_back(glm::vec4(x * offset + (randomXOffset / 100.0f), 0.0f, y * offset + (randomZOffset / 100.0f), 1.0f));
            }
        }
    }

    glGenBuffers(1, &this->instancedVBO_ID);
}

void cInstancedRenderer::SaveOffsets()
{
    std::ofstream outputFile(fileName);
    if (outputFile.is_open())
    {
        for (size_t i = 0; i < this->translations.size(); i++)
        {
            glm::vec4 offset = this->translations[i];
            outputFile << offset.x << "," << offset.y << "," << offset.z;

            if (i < this->translations.size() - 1)
                outputFile << std::endl;
        }
    }
    outputFile.close();
}

void cInstancedRenderer::SetupVertexArrayAttrib(sModelDrawInfo* drawInfo)
{
    glBindVertexArray(drawInfo->VAO_ID);
    glBindBuffer(GL_ARRAY_BUFFER, this->instancedVBO_ID);

    if(this->translations.size() > 0)
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