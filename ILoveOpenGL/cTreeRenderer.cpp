#include "cTreeRenderer.h"
#include "GLCommon.h"

cTreeRenderer::cTreeRenderer(unsigned int treeAmount, float offset)
{
	this->treeAmount = treeAmount;
	this->offset = offset;

    int amount = (int)glm::sqrt(treeAmount);
    for (int y = -amount; translations.size() < treeAmount; y += 2)
    {
        for (int x = -amount; x < amount; x += 2)
        {
            translations.push_back(glm::vec4(x * offset, 0.0f, y * offset, 1.0f));
        }
    }
}

unsigned int cTreeRenderer::GetTreeCount()
{
    return this->treeAmount;
}
bool cTreeRenderer::GetUniformSetupComplete()
{
    return this->uniformSetupComplete;
}

void cTreeRenderer::SetupUniformLocations(unsigned int shaderId)
{
    for (unsigned int i = 0; i < this->treeAmount; i++)
    {
        std::string name = "offsets[" + std::to_string(i) + "]";
        glm::vec4 curOffset = this->translations[i];

        GLint uniformLocation = glGetUniformLocation(shaderId, name.c_str());
        glUniform4f(uniformLocation, curOffset.x, curOffset.y, curOffset.z, curOffset.w);
    }

    this->uniformSetupComplete = true;
}