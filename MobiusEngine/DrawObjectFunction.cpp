#include "Components/cMeshRenderer.h" // glm::mat4
#include "GLCommon.h"
#include "Managers/cVAOManager.h"
#include <glm/glm.hpp>
#include <glm/mat4x4.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "Managers/cBasicTextureManager/cBasicTextureManager.h"
#include "Components/cTransform.h"
#include "Components/cTextureViewer.h"
#include "RenderType.h"
#include "Managers/cShaderManager.h"
#include "Components/cInstancedRenderer.h"

void SetUpTextures(cEntity* curEntity, cBasicTextureManager textureManager, std::map<std::string, GLint>* uniformLocations)
{
    cMeshRenderer* curMesh = curEntity->GetComponent<cMeshRenderer>();
    cTextureViewer* curTextureViewer = curEntity->GetComponent<cTextureViewer>();

    float ratioOne = curMesh->textures[0].ratio;

    if (curTextureViewer != nullptr)
    {
        ratioOne = 1.0f;
    }

    glUniform4f(uniformLocations->at("textureRatios"),
        curMesh->useAlbedoMap ? 1.0f : 0.0f, 0.0f, 0.0f, 0.0f);
    glUniform4f(uniformLocations->at("tilingAndOffset"), curMesh->tiling.x, curMesh->tiling.y, curMesh->offset.x, curMesh->offset.y);

    if (curMesh->bUseAlphaMask)
    {
        glUniform1f(uniformLocations->at("bUseAlphaMask"), (float)GL_TRUE);

        GLint textureId = textureManager.getTextureIDFromName(curMesh->alphaMaskName);
        if (textureId != 0)
        {
            GLint unit = 1;
            glActiveTexture(unit + GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, textureId);
            glUniform1i(uniformLocations->at("alphaMask"), unit);
        }
    }
    else
    {
        glUniform1f(uniformLocations->at("bUseAlphaMask"), (float)GL_FALSE);
    }

    if (curMesh->useMetallicMap)
    {
        glUniform1f(uniformLocations->at("bUseMetallicMap"), (float)GL_TRUE);

        GLint textureId = textureManager.getTextureIDFromName(curMesh->metallicMapName);
        if (textureId != 0)
        {
            GLint unit = 1;
            glActiveTexture(unit + GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, textureId);
            glUniform1i(uniformLocations->at("metallicMap"), unit);
        }
    }
    else
    {
        glUniform1f(uniformLocations->at("bUseMetallicMap"), (float)GL_FALSE);
    }

    if (curMesh->useAOMap)
    {
        glUniform1f(uniformLocations->at("bUseAO"), (float)GL_TRUE);

        GLint textureId = textureManager.getTextureIDFromName(curMesh->AOMapName);
        if (textureId != 0)
        {
            GLint unit = 1;
            glActiveTexture(unit + GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, textureId);
            glUniform1i(uniformLocations->at("AOMap"), unit);
        }
    }
    else
    {
        glUniform1f(uniformLocations->at("bUseAO"), (float)GL_FALSE);
    }

    if (curMesh->useRoughnessMap)
    {
        glUniform1f(uniformLocations->at("bUseRoughMap"), (float)GL_TRUE);

        GLint textureId = textureManager.getTextureIDFromName(curMesh->roughnessMapName);
        if (textureId != 0)
        {
            GLint unit = 1;
            glActiveTexture(unit + GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, textureId);
            glUniform1i(uniformLocations->at("roughMap"), unit);
        }
    }
    else
    {
        glUniform1f(uniformLocations->at("bUseRoughMap"), (float)GL_FALSE);
    }

    if (curMesh->bUseNormalMap)
    {
        glUniform1f(uniformLocations->at("bUseNormalMap"), (float)GL_TRUE);

        GLint textureId = textureManager.getTextureIDFromName(curMesh->normalMapName);
        if (textureId != 0)
        {
            GLint unit = 2;
            glActiveTexture(unit + GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, textureId);

            glUniform1i(uniformLocations->at("normalMap"), unit);
        }
    }
    else
    {
        glUniform1f(uniformLocations->at("bUseNormalMap"), (float)GL_FALSE);
    }


    if (curMesh->bUseHeightMap)
    {
        glUniform1f(uniformLocations->at("bUseHeightMap"), (float)GL_TRUE);
        GLint textureId = textureManager.getTextureIDFromName(curMesh->heightMapName);
        if (textureId != 0)
        {
            GLint unit = 3;
            glActiveTexture(unit + GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, textureId);
            //glTexParameteri(textureId, GL_TEXTURE_WRAP_S, GL_REPEAT);
            //glTexParameteri(textureId, GL_TEXTURE_WRAP_T, GL_REPEAT);
            glUniform1i(uniformLocations->at("heightMap"), unit);
        }
    }
    else
    {
        glUniform1f(uniformLocations->at("bUseHeightMap"), (float)GL_FALSE);
    }

    if (curMesh->bUseSkybox)
    {
        glUniform1f(uniformLocations->at("bUseSkybox"), (float)GL_TRUE);

        GLint textureId = textureManager.getTextureIDFromName(curMesh->textures[0].name);
        if (textureId != 0)
        {
            //Make cubemap unit 20 so they don't overlap with normal textures
            GLint unit = 4;
            glActiveTexture(unit + GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_CUBE_MAP, textureId);
            glUniform1i(uniformLocations->at("skyBox"), unit);
        }
    }
    else
    {
        glUniform1f(uniformLocations->at("bUseSkybox"), (float)GL_FALSE);
        if (ratioOne > 0.0f)
        {
            GLint textureId = 0;

            if (curTextureViewer != nullptr)
            {
                textureId = curTextureViewer->GetCurrentTextId();
            }
            else
            {
                textureId = textureManager.getTextureIDFromName(curMesh->textures[0].name);
            }

            if (textureId != 0)
            {
                GLint unit = 5;
                glActiveTexture(unit + GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, textureId);
                glUniform1i(uniformLocations->at("texture_00"), unit);
            }
        }
        /*if (ratioFour > 0.0f)
        {

        }*/
    }
}

void Render(sModelDrawInfo* modelInfo, cEntity* curEntity, cShaderManager::cShaderProgram* shader)
{
    glBindVertexArray(modelInfo->VAO_ID);
    cInstancedRenderer* instancedRenderer = curEntity->GetComponent<cInstancedRenderer>();
    if (instancedRenderer == nullptr)
    {
        //err = glGetError();
        glUniform1f(shader->uniformLocations["bUseInstancedRendering"], (float)GL_FALSE);
        glDrawElements(GL_TRIANGLES,
            modelInfo->numberOfIndices,
            GL_UNSIGNED_INT,
            (void*)0);
    }
    else
    {
        glUniform1f(shader->uniformLocations["bUseInstancedRendering"], (float)GL_TRUE);
        glDrawElementsInstanced(GL_TRIANGLES,
            modelInfo->numberOfIndices,
            GL_UNSIGNED_INT,
            (void*)0, (GLsizei)instancedRenderer->GetCount());

    }

    glBindVertexArray(0);

    //for (sModelDrawInfo child : modelInfo->children)
    //{
    //    Render(&child, curEntity, shader);
    //}
}

void DrawObject(cEntity* curEntity, glm::mat4 matModel, cShaderManager::cShaderProgram* shader, cVAOManager* VAOManager,
    cBasicTextureManager textureManager,  glm::vec3 eyeLocation, sModelDrawInfo* model = nullptr)
{
    cMeshRenderer* curMesh = curEntity->GetComponent<cMeshRenderer>();
    if (!curMesh->render)
    {
        return;
    }

    cTransform* curTransform = curEntity->GetComponent<cTransform>();
    if (shader->type == RenderType::Normal)
    {
        SetUpTextures(curEntity, textureManager, &shader->uniformLocations);
    }
    // *****************************************************
            // Translate or "move" the object somewhere
    glm::mat4 matTranslate = glm::translate(glm::mat4(1.0f),
        curTransform->position);

    //matModel = matModel * matTranslate;
    // *****************************************************


    // *****************************************************
    // Rotation around the Z axis
    glm::mat4 matRotation = glm::toMat4(curTransform->GetQuatRotation());

    //matModel = matModel * rotateX;
    // *****************************************************

    // *****************************************************
    // Scale the model
    glm::mat4 matScale = glm::scale(glm::mat4(1.0f),
        curTransform->scale);// Scale in Z

//matModel = matModel * matScale;
// *****************************************************

// *****************************************************
    matModel = matModel * matTranslate;
    matModel = matModel * matRotation;
    matModel = matModel * matScale;     // <-- mathematically, this is 1st

    GLint matModel_Location = shader->uniformLocations["matModel"];
    glUniformMatrix4fv(matModel_Location, 1, GL_FALSE, glm::value_ptr(matModel));

    if (shader->type != RenderType::Shadow)
    {
        GLint matModelInverseTranspose_Location = shader->uniformLocations["matModelInverseTranspose"];
        glm::mat4 matInvTransposeModel = glm::inverse(glm::transpose(matModel));
        glUniformMatrix4fv(matModelInverseTranspose_Location, 1, GL_FALSE, glm::value_ptr(matInvTransposeModel));
    }
    else if (shader->type == RenderType::Shadow)
    {
        glUniform1f(shader->uniformLocations["bUseWind"], curMesh->useWind ? (float)GL_TRUE : (float)GL_FALSE);
        if (curMesh->bUseAlphaMask)
        {
            glUniform1f(shader->uniformLocations.at("bUseAlphaMask"), (float)GL_TRUE);

            GLint textureId = textureManager.getTextureIDFromName(curMesh->alphaMaskName);
            if (textureId != 0)
            {
                GLint unit = 1;
                glActiveTexture(unit + GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, textureId);
                glUniform1i(shader->uniformLocations.at("alphaMask"), unit);
            }
        }
        else
        {
            glUniform1f(shader->uniformLocations.at("bUseAlphaMask"), (float)GL_FALSE);
        }
    }

    if (shader->type == RenderType::PingPong || shader->type == RenderType::Shadow)
    {
        sModelDrawInfo modelInfo;
        if (model == nullptr)
        {
            if (VAOManager->FindDrawInfoByModelName(curMesh->meshName, modelInfo))
            {
                Render(&modelInfo, curEntity, shader);
            }

            for (size_t i = 0; i < curEntity->children.size(); i++)
            {
                DrawObject(curEntity->children[i], matModel, shader, VAOManager, textureManager, eyeLocation, &modelInfo.children[i]);
            }
        }
        else
        {
            Render(model, curEntity, shader);
        }
        return;
    }

    if (curMesh->bIsImposter)
    {
        glUniform1f(shader->uniformLocations["bIsImposter"], (float)GL_TRUE);
    }
    else
    {
        glUniform1f(shader->uniformLocations["bIsImposter"], (float)GL_FALSE);
    }

    if (curMesh->bUseWholeObjectDiffuseColour)
    {
        glUniform1f(shader->uniformLocations["bUseWholeObjectDiffuseColour"], (float)GL_TRUE);
        glUniform4f(shader->uniformLocations["wholeObjectDiffuseColour"],
            curMesh->wholeObjectDiffuseRGBA.r,
            curMesh->wholeObjectDiffuseRGBA.g,
            curMesh->wholeObjectDiffuseRGBA.b,
            curMesh->wholeObjectDiffuseRGBA.a);
    }
    else
    {
        glUniform1f(shader->uniformLocations["bUseWholeObjectDiffuseColour"], (float)GL_FALSE);
    }

    glUniform4f(shader->uniformLocations["wholeObjectSpecularColour"],
        curMesh->wholeObjectSpecularRGB.r,
        curMesh->wholeObjectSpecularRGB.g,
        curMesh->wholeObjectSpecularRGB.b,
        curMesh->wholeObjectShininess_SpecPower);

    glUniform3f(shader->uniformLocations["emmision"], curMesh->emmisionDiffuse.r * curMesh->emmisionPower, curMesh->emmisionDiffuse.g * curMesh->emmisionPower, curMesh->emmisionDiffuse.b * curMesh->emmisionPower);
    glUniform1f(shader->uniformLocations["bUseWind"], curMesh->useWind ? (float)GL_TRUE : (float) GL_FALSE);
    glUniform1f(shader->uniformLocations["bIsPlane"], curMesh->useWind ? (float)GL_TRUE : (float) GL_FALSE);
    glUniform1f(shader->uniformLocations["brightness"], curMesh->diffuseBrightness);

    glUniform1f(shader->uniformLocations["roughness"], curMesh->roughness);
    glUniform1f(shader->uniformLocations["metallic"], curMesh->metallic);

    // See if mesh is wanting the vertex colour override (HACK) to be used?
    if (curMesh->bUseObjectDebugColour)
    {
        // Override the colour...
        glUniform1f(shader->uniformLocations["bUseDebugColour"], (float)GL_TRUE);
        glUniform4f(shader->uniformLocations["objectDebugColour"],
            curMesh->objectDebugColourRGBA.r,
            curMesh->objectDebugColourRGBA.g,
            curMesh->objectDebugColourRGBA.b,
            curMesh->objectDebugColourRGBA.a);
    }
    else
    {
        // DON'T override the colour
        glUniform1f(shader->uniformLocations["bUseDebugColour"], (float)GL_FALSE);
    }

    // See if mesh is wanting the vertex colour override (HACK) to be used?
    if (curMesh->bDontLight)
    {
        // Override the colour...
        glUniform1f(shader->uniformLocations["bDontLightObject"], (float)GL_TRUE);
    }
    else
    {
        // DON'T override the colour
        glUniform1f(shader->uniformLocations["bDontLightObject"], (float)GL_FALSE);
    }

    if (curMesh->bUseSpecular)
    {
        // Override the colour...
        glUniform1f(shader->uniformLocations["bUseSpecular"], (float)GL_TRUE);
    }
    else
    {
        // DON'T override the colour
        glUniform1f(shader->uniformLocations["bUseSpecular"], (float)GL_FALSE);
    }

    if (curMesh->bUseSkyboxReflection)
    {
        // Override the colour...
        glUniform1f(shader->uniformLocations["bUseSkyboxReflections"], (float)GL_TRUE);
    }
    else
    {
        // DON'T override the colour
        glUniform1f(shader->uniformLocations["bUseSkyboxReflections"], (float)GL_FALSE);
    }

    if (curMesh->bUseSkyboxRefraction)
    {
        // Override the colour...
        glUniform1f(shader->uniformLocations["bUseSkyboxRefraction"], (float)GL_TRUE);
    }
    else
    {
        // DON'T override the colour
        glUniform1f(shader->uniformLocations["bUseSkyboxRefraction"], (float)GL_FALSE);
    }

    // Wireframe
    if (curMesh->bIsWireframe)                // GL_POINT, GL_LINE, and GL_FILL)
    {
        // Draw everything with only lines
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    }
    else
    {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }
    sModelDrawInfo modelInfo;
    //        if (gVAOManager.FindDrawInfoByModelName("bun_zipper_res2 (justXYZ).ply", modelInfo))
    //        if (gVAOManager.FindDrawInfoByModelName("Assembled_ISS.ply", modelInfo))

    if (model == nullptr)
    {
        if (VAOManager->FindDrawInfoByModelName(curMesh->meshName, modelInfo))
        {
            Render(&modelInfo, curEntity, shader);
        }

        for (size_t i = 0; i < curEntity->children.size(); i++)
        {
            DrawObject(curEntity->children[i], matModel, shader, VAOManager, textureManager, eyeLocation, &modelInfo.children[i]);
        }
    }

    //Child rendering
    else
    {
        Render(model, curEntity, shader);
    }
}