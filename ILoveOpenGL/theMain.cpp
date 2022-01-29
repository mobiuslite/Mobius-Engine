//#include <glad/glad.h>
//
//#define GLFW_INCLUDE_NONE
//#include <GLFW/glfw3.h>

#include "GLCommon.h"

//#include "linmath.h"
#include <glm/glm.hpp>
#include <glm/vec3.hpp> // glm::vec3
#include <glm/vec4.hpp> // glm::vec4
#include <glm/mat4x4.hpp> // glm::mat4
#include <glm/gtc/matrix_transform.hpp> 
// glm::translate, glm::rotate, glm::scale, glm::perspective
#include <glm/gtc/type_ptr.hpp> // glm::value_ptr


#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <string>
#include <vector>       // "smart array"
#include <fstream>      // C++ file I-O library (look like any other stream)

#include "cVAOManager.h"
#include "cShaderManager.h"

#include "cLightManager.h"
#include "cLightHelper.h"
#include "SceneLoader/cSceneLoader.h"
#include "SceneLoader/sModel.h"

#include "FlyCamera/cFlyCamera.h"
#include "cBasicTextureManager/cBasicTextureManager.h"
#include <algorithm>
#include "Physics/cWorld.h"
#include "cEmitters.h"
#include "Physics/cForceGenerator.h"

#include "cFBO/cFBO.h"
#include "cMeshRenderer.h"
#include "cEntityManager.h"
#include "cTransform.h"

enum class Transform
{
    Translate,
    Rotate,
    Scale
};

GLuint program;
cFBO* g_fbo;

// Global things are here:

cFlyCamera* g_FlyCamera = NULL;
float flyCameraSpeed = 5.0f;
bool g_MouseIsInsideWindow = false;

cVAOManager     gVAOManager;
cShaderManager  gShaderManager;

cEntityManager g_entityManager;

cLightManager gTheLights;
cLightHelper gTheLightHelper;

cBasicTextureManager g_textureManager;

cEntity* g_DebugSphere = NULL;

unsigned int g_selectedObject = 0;
unsigned int g_selectedLight = 0;

Transform transformType = Transform::Translate;

cSceneLoader* sceneLoader;
std::string sceneName = "project2";

std::vector<cEntity*> clouds;
bool isNightTime = true;

bool isDebugMode = false;
bool debugShowLighting = true;
bool debugShowNormals = false;

cEntity* g_skyBox;

bool preWarm = true;

//Method in DrawObjectFunction
void extern DrawObject(cEntity* curEntity, glm::mat4 matModel, GLint program, cVAOManager* VAOManager,
    cBasicTextureManager textureManager, std::map<std::string, GLint> uniformLocations, glm::vec3 eyeLocation);

static void GLFW_cursor_enter_callback(GLFWwindow* window, int entered)
{
    if (entered)
    {
       // std::cout << "Mouse cursor is over the window" << std::endl;
        ::g_MouseIsInsideWindow = true;
    }
    else
    {
        //std::cout << "Mouse cursor is no longer over the window" << std::endl;
        ::g_MouseIsInsideWindow = false;
    }
    return;
}
static void GLFW_scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    float mouseScrollWheelSensitivity = 0.1f;

    ::g_FlyCamera->setMouseWheelDelta(yoffset * mouseScrollWheelSensitivity);

    return;

}
static void error_callback(int error, const char* description)
{
    fprintf(stderr, "Error: %s\n", description);
}

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
    {
        glfwSetWindowShouldClose(window, GLFW_TRUE);
    }
    else if (key == GLFW_KEY_F1 && action == GLFW_PRESS)
    {
        isDebugMode = !isDebugMode;

        if (isDebugMode)
        {
            std::cout << "Debug mode: ENABLED" << std::endl;
        }
        else
        {
            std::cout << "Debug mode: DISABLED" << std::endl;
        }
    }
    else if (key == GLFW_KEY_F3 && action == GLFW_PRESS)
    {
        debugShowLighting = !debugShowLighting;
    }
    else if (key == GLFW_KEY_F4 && action == GLFW_PRESS)
    {
        debugShowNormals = !debugShowNormals;
    }
    else if (key == GLFW_KEY_N && action == GLFW_PRESS)
    {
        isNightTime = !isNightTime;

        if (isNightTime)
        {
            gTheLights.TurnOffLight(8);
            gTheLights.TurnOffLight(12);

            gTheLights.TurnOnLight(0);
            gTheLights.TurnOnLight(1);
            gTheLights.TurnOnLight(2);
            gTheLights.TurnOnLight(3);
            gTheLights.TurnOnLight(4);
            gTheLights.TurnOnLight(5);
            gTheLights.TurnOnLight(6);
            gTheLights.TurnOnLight(7);
            gTheLights.TurnOnLight(9);

            g_skyBox->GetComponent<cMeshRenderer>()->textures[0].name = "NightSky";
        }
        else
        {
            gTheLights.TurnOnLight(8);
            gTheLights.TurnOnLight(12);

            gTheLights.TurnOffLight(0);
            gTheLights.TurnOffLight(1);
            gTheLights.TurnOffLight(2);
            gTheLights.TurnOffLight(3);
            gTheLights.TurnOffLight(4);
            gTheLights.TurnOffLight(5);
            gTheLights.TurnOffLight(6);
            gTheLights.TurnOffLight(7);
            gTheLights.TurnOffLight(9);

            g_skyBox->GetComponent<cMeshRenderer>()->textures[0].name = "TropicalSunnyDay";
        }
    }
    else if (key == GLFW_KEY_SPACE && action == GLFW_PRESS)
    {
        if (sceneLoader->SaveScene(sceneName, g_FlyCamera->getEye(), &g_entityManager))
        {
            std::cout << "Saved scene: " << sceneName << std::endl;
        }
    }
    else if (key == GLFW_KEY_C && action == GLFW_PRESS)
    {
        cEntity* cloneMesh = g_entityManager.GetEntities().at(g_selectedObject)->clone();
        cloneMesh->GetComponent<cMeshRenderer>()->friendlyName += "Clone";

        cEntity* newEntity = g_entityManager.CreateEntity();
        *newEntity = *cloneMesh;

        g_entityManager.DeleteEntity(cloneMesh);

        g_selectedObject = (int)g_entityManager.GetEntities().size() - 1;

        std::cout << "Cloned mesh!" << std::endl;

    }
    else if (key == GLFW_KEY_INSERT && action == GLFW_PRESS)
    {

        //ADDING NEW OR EXISTING MODELS.
        std::string type;
        std::string param;

        std::cout << "What would you like do to? (add/del): ";

        std::cin >> type;
        std::cout << std::endl;

        if (type == "add")
        {
            std::cout << "Add mesh from existing model? (y/n): ";
            std::cin >> type;

            //Add a new mesh from a model already loaded into the VAO
            if (type == "y")
            {
                std::cout << std::endl;
                std::cout << "Add a friendly name for the object: ";
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

                std::string name;
                std::getline(std::cin, name);

                std::cout << std::endl;
                std::cout << "Select a model to create a mesh from (1-" << (*sceneLoader->GetModels()).size() << "): " << std::endl;

                //Prints out all models being used.
                int i = 1;
                for (sModel model : *sceneLoader->GetModels())
                {
                    std::cout << "\t" << i << "." << model.fileName << std::endl;

                    i++;
                }

                std::cout << std::endl;

                std::string selectionString;
                std::cin >> selectionString;
                int selection = -1;

                //Attempts to get the user's response
                try
                {

                    selection = std::stoi(selectionString) - 1;
                }
                catch (const std::exception& e)
                {
                    std::cout << "Please use numbers to select a model" << std::endl;
                    std::cout << e.what() << std::endl;
                }


                //Creates a new mesh and adds it to the list of meshes if the response of good!
                cMeshRenderer* newMesh = new cMeshRenderer();
                cTransform newTransform;

                try
                {
                    sModel selectedModel = (*sceneLoader->GetModels()).at(selection);

                    newMesh->meshName = selectedModel.fileName;
                    newTransform.scale = glm::vec3(selectedModel.defaultScale);
                    newMesh->friendlyName = name;
                }
                catch (const std::exception& e)
                {
                    std::cout << "Selection out of bound" << std::endl;
                    std::cout << e.what() << std::endl;
                }

                //Adds the new mesh to the list of meshes to render
                if (newMesh->meshName != "")
                {
                    cEntity* newEntity = g_entityManager.CreateEntity();
                    newEntity->AddComponent<cMeshRenderer>(newMesh);
                    *newEntity->GetComponent<cTransform>() = newTransform;

                    g_selectedObject = (int)g_entityManager.GetEntities().size() - 1;

                    std::cout << "Added new mesh from model: " << newMesh->meshName << std::endl;
                }

            }
            //If you want to import a new model
            else if (type == "n")
            {

                std::cout << std::endl;
                std::cout << "Enter the name of the file (e.g. bunny.ply | make sure it's in the \"assets\\models\" folder): ";
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

                std::string fileName;
                std::getline(std::cin, fileName);

                sModel newModel;
                sModelDrawInfo modelDrawInfo;

                //Attempts to load the model into the vao
                if (gVAOManager.LoadModelIntoVAO(fileName, modelDrawInfo, program))
                {
                    std::cout << "Loaded " << fileName << " into the VAO!" << std::endl;

                    newModel.fileName = fileName;
                    newModel.defaultScale = modelDrawInfo.defaultScale;

                    (*sceneLoader->GetModels()).push_back(newModel);

                    std::cout << std::endl;
                    std::cout << "Add a friendly name for the object: ";

                    std::string name;
                    std::getline(std::cin, name);


                    //Adds the new mesh to the screen.
                    cMeshRenderer* newMesh = new cMeshRenderer();
                    cTransform newTransform;

                    newMesh->meshName = newModel.fileName;
                    newTransform.scale = glm::vec3(newModel.defaultScale);
                    newMesh->friendlyName = name;

                    cEntity* newEntity = g_entityManager.CreateEntity();

                    *newEntity->GetComponent<cTransform>() = newTransform;
                    newEntity->AddComponent<cMeshRenderer>(newMesh);

                    g_selectedObject = (int)g_entityManager.GetEntities().size() - 1;

                    std::cout << "Added new mesh from model: " << newMesh->meshName << std::endl;
                }
                else
                {
                    std::cout << "ERROR: Could not load " << fileName << " into the VAO" << std::endl;
                }

            }
        }

        //Deleting a mesh from the scene
        else if (type == "del")
        {
            std::vector<cEntity*> entities = g_entityManager.GetEntities();

            std::cout << std::endl;
            std::cout << "Select a mesh to delete (1-" << entities.size() << "): " << std::endl;

            //Prints out all models being used.
            int i = 1;
            for (cEntity* model : entities)
            {
                std::cout << "\t" << i << "." << model->GetComponent<cMeshRenderer>()->friendlyName << std::endl;

                i++;
            }

            std::cout << std::endl;

            std::string selectionString;
            std::cin >> selectionString;
            int selection = -1;

            //Attempts to get the user's response
            try
            {

                selection = std::stoi(selectionString) - 1;
            }
            catch (const std::exception& e)
            {
                std::cout << "Please use numbers to select a mesh" << std::endl;
                std::cout << e.what() << std::endl;
            }

            try
            {
                cEntity* selectedSceneEntity = entities.at(selection);
                cMeshRenderer selectedMesh = *selectedSceneEntity->GetComponent<cMeshRenderer>();

                g_entityManager.RemoveEntity(selection);

                std::cout << "Deleted mesh " << selectedMesh.friendlyName << std::endl;
                g_selectedObject = (int)g_entityManager.GetEntities().size() - 1;
            }
            catch (const std::exception& e)
            {
                std::cout << "Selection out of bound" << std::endl;
                std::cout << e.what() << std::endl;
            }

        }
        else
        {
            std::cout << "That is not a valid option" << std::endl << std::endl;
        }
    }

    bool bShiftDown = false;
    bool bControlDown = false;
    bool bAltDown = false;

//    // Shift down?
//    if ( mods == GLFW_MOD_SHIFT )       // 0x0001   0000 0001
//    {
//        // ONLY shift is down
//    }
//    // Control down?
//    if ( mods == GLFW_MOD_CONTROL  )    // 0x0002   0000 0010
//    // Alt down?
//    if ( mods == GLFW_MOD_ALT   )       // 0x0004   0000 0100

    //   0000 0111 
    // & 0000 0001
    // -----------
    //   0000 0001 --> Same as the shift mask
    
    // Use bitwise mask to filter out just the shift
    if ( (mods & GLFW_MOD_SHIFT) == GLFW_MOD_SHIFT)
    {
        // Shift is down and maybe other things, too
        bShiftDown = true;
    }
    if ( (mods & GLFW_MOD_CONTROL) == GLFW_MOD_CONTROL)
    {
        // Shift is down and maybe other things, too
        bControlDown = true;
    }
    if ( (mods & GLFW_MOD_ALT) == GLFW_MOD_ALT)
    {
        // Shift is down and maybe other things, too
        bAltDown = true;
    }

    // If you are using a bunch of combos, maybe make a set of 
    //  functions like "isShiftDownByItself()" and "isShiftDown()", etc.
    float cameraSpeed = 0.1f;
    float lightMovementSpeed = .25f;
    float moveSpeed = 0.05f;
    float rotateSpeed = 1.0f;
    float scaleSpeed = 0.015f;

    //Fixes scale speed;
    for (int i = 0; i < sceneLoader->GetModels()->size(); i++)
    {
        if (sceneLoader->GetModels()->at(i).fileName == g_entityManager.GetEntities().at(g_selectedObject)->GetComponent<cMeshRenderer>()->meshName)
        {
            scaleSpeed *= sceneLoader->GetModels()->at(i).defaultScale;
        }
    }

    // If JUST the shift is down, move the "selected" object
    if ( bShiftDown && ( ! bControlDown ) && ( ! bAltDown ) )
    {
        if (transformType == Transform::Translate)
        {
            if (key == GLFW_KEY_A) { g_entityManager.GetEntities().at(::g_selectedObject)->GetComponent<cTransform>()->position.x -= moveSpeed; } // Go left
            if (key == GLFW_KEY_D) { g_entityManager.GetEntities().at(::g_selectedObject)->GetComponent<cTransform>()->position.x += moveSpeed; } // Go right
            if (key == GLFW_KEY_W) { g_entityManager.GetEntities().at(::g_selectedObject)->GetComponent<cTransform>()->position.z += moveSpeed; }// Go forward 
            if (key == GLFW_KEY_S) { g_entityManager.GetEntities().at(::g_selectedObject)->GetComponent<cTransform>()->position.z -= moveSpeed; }// Go backwards
            if (key == GLFW_KEY_Q) { g_entityManager.GetEntities().at(::g_selectedObject)->GetComponent<cTransform>()->position.y -= moveSpeed; }// Go "Down"
            if (key == GLFW_KEY_E) { g_entityManager.GetEntities().at(::g_selectedObject)->GetComponent<cTransform>()->position.y += moveSpeed; }// Go "Up"
        }
        else if (transformType == Transform::Rotate)
        {
            glm::vec3 euler = g_entityManager.GetEntities().at(::g_selectedObject)->GetComponent<cTransform>()->GetEulerRotation();

            if (key == GLFW_KEY_A) { euler.x -= glm::radians(rotateSpeed); } // Go left
            if (key == GLFW_KEY_D) { euler.x += glm::radians(rotateSpeed); } // Go right
            if (key == GLFW_KEY_W) { euler.z += glm::radians(rotateSpeed); }// Go forward 
            if (key == GLFW_KEY_S) { euler.z -= glm::radians(rotateSpeed); }// Go backwards
            if (key == GLFW_KEY_Q) { euler.y -= glm::radians(rotateSpeed); }// Go "Down"
            if (key == GLFW_KEY_E) { euler.y += glm::radians(rotateSpeed); }// Go "Up"

            g_entityManager.GetEntities().at(::g_selectedObject)->GetComponent<cTransform>()->SetRotation(euler);
        }
        else if (transformType == Transform::Scale)
        {
            if (key == GLFW_KEY_W)
            {
                g_entityManager.GetEntities().at(::g_selectedObject)->GetComponent<cTransform>()->scale += scaleSpeed;
            }
            else if (key == GLFW_KEY_S)
            {
                g_entityManager.GetEntities().at(::g_selectedObject)->GetComponent<cTransform>()->scale -= scaleSpeed;
            }
        }
    // TODO: Add some controls to change the "selcted object"
    // i.e. change the ::g_selectedObject value


    }//if ( bShiftDown && ( ! bControlDown ) && ( ! bAltDown ) )

    if (key == GLFW_KEY_R && action == GLFW_PRESS)
    {
        transformType = Transform::Translate;
    }
    else if (key == GLFW_KEY_T && action == GLFW_PRESS)
    {
        transformType = Transform::Rotate;
    }
    else if (key == GLFW_KEY_Y && action == GLFW_PRESS)
    {
        transformType = Transform::Scale;
    }

    // If JUST the ALT is down, move the "selected" light
    if ( ( ! bShiftDown ) && ( ! bControlDown ) && bAltDown )
    {
        if (key == GLFW_KEY_A)  {   ::gTheLights.theLights[::g_selectedLight].position.x -= lightMovementSpeed;     } // Go left
        if (key == GLFW_KEY_D)  {   ::gTheLights.theLights[::g_selectedLight].position.x += lightMovementSpeed;     } // Go right
        if (key == GLFW_KEY_W)  {   ::gTheLights.theLights[::g_selectedLight].position.z += lightMovementSpeed;     }// Go forward 
        if (key == GLFW_KEY_S)  {   ::gTheLights.theLights[::g_selectedLight].position.z -= lightMovementSpeed;     }// Go backwards
        if (key == GLFW_KEY_Q)  {   ::gTheLights.theLights[::g_selectedLight].position.y -= lightMovementSpeed;     }// Go "Down"
        if (key == GLFW_KEY_E)  {   ::gTheLights.theLights[::g_selectedLight].position.y += lightMovementSpeed;     }// Go "Up"

        // constant attenuation
        if (key == GLFW_KEY_1 ) 
        {  
            ::gTheLights.theLights[::g_selectedLight].atten.x *= 0.99f; // -1% less
        }
        else if (key == GLFW_KEY_2 ) 
        {  
            ::gTheLights.theLights[::g_selectedLight].atten.x *= 1.01f; // +1% more
        }
        // linear attenuation
        else if (key == GLFW_KEY_3 )
        {  
            ::gTheLights.theLights[::g_selectedLight].atten.y *= 0.99f; // -1% less
        }
        else if (key == GLFW_KEY_4 )
        {  
            ::gTheLights.theLights[::g_selectedLight].atten.y *= 1.01f; // +1% more
        }
        // quardatic attenuation
        else if (key == GLFW_KEY_5 )
        {  
            ::gTheLights.theLights[::g_selectedLight].atten.z *= 0.99f; // -1% less
        }
        else if (key == GLFW_KEY_6 )
        {  
            ::gTheLights.theLights[::g_selectedLight].atten.z *= 1.01f; // +1% more
        }

        if (key == GLFW_KEY_PAGE_UP)
        {
            ::gTheLights.theLights[::g_selectedLight].param2.x = 1.0f;
        }
        else if (key == GLFW_KEY_PAGE_DOWN)
        {
            ::gTheLights.theLights[::g_selectedLight].param2.x = 0.0f;
        }

        if (key == GLFW_KEY_PERIOD && action == GLFW_PRESS)
        {
            if(g_selectedLight >= 1)
                g_selectedLight--;
        }
        else if (key == GLFW_KEY_SLASH && action == GLFW_PRESS)
        {
            if (g_selectedLight < cLightManager::NUMBER_OF_LIGHTS - 1)
                g_selectedLight++;
        }

        std::cout << ::g_selectedLight << " positionXYZ : "
            << ::gTheLights.theLights[::g_selectedLight].position.x << ", "
            << ::gTheLights.theLights[::g_selectedLight].position.y << ", "
            << ::gTheLights.theLights[::g_selectedLight].position.z << "  "
            << "attenuation (C, L, Q): " 
            << ::gTheLights.theLights[::g_selectedLight].atten.x << ", "        // Const
            << ::gTheLights.theLights[::g_selectedLight].atten.y << ", "        // Linear
            << ::gTheLights.theLights[::g_selectedLight].atten.z << "  "        // Quadratic
            << std::endl;

    // TODO: Add some controls to change the "selcted object"
    // i.e. change the ::g_selectedObject value


    }//if ( bShiftDown && ( ! bControlDown ) && ( ! bAltDown ) )


    //Changing selected mesh
    else if (key == GLFW_KEY_SEMICOLON && action == GLFW_PRESS && g_selectedObject > 0)
    {
        g_selectedObject--;
        std::cout << "Selected mesh: " << g_entityManager.GetEntities().at(::g_selectedObject)->GetComponent<cMeshRenderer>()->friendlyName << std::endl;
    }
    else if (key == GLFW_KEY_APOSTROPHE && action == GLFW_PRESS && g_selectedObject < g_entityManager.GetEntities().size() - 1)
    {
        g_selectedObject++;
        std::cout << "Selected mesh: " << g_entityManager.GetEntities().at(::g_selectedObject)->GetComponent<cMeshRenderer>()->friendlyName << std::endl;
    }

    return;
}

bool DistanceToCameraPredicate(cEntity* a, cEntity* b)
{
    if (glm::distance(a->GetComponent<cTransform>()->position, g_FlyCamera->getEye()) > glm::distance(b->GetComponent<cTransform>()->position, g_FlyCamera->getEye()))
    {
        return true;
    }
    else
    {
        return false;
    }
}

// We call these every frame
void ProcessAsyncMouse(GLFWwindow* window, float deltaTime)
{

    double x, y;
    glfwGetCursorPos(window, &x, &y);

    ::g_FlyCamera->setMouseXY(x, y);

    const float MOUSE_SENSITIVITY = 4.0f;


    // Mouse left (primary?) button pressed? 
    // AND the mouse is inside the window...
    if ((glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
        && ::g_MouseIsInsideWindow)
    {
        // Mouse button is down so turn the camera
        ::g_FlyCamera->Yaw_LeftRight(-::g_FlyCamera->getDeltaMouseX() * MOUSE_SENSITIVITY * deltaTime);

        ::g_FlyCamera->Pitch_UpDown(::g_FlyCamera->getDeltaMouseY() * MOUSE_SENSITIVITY * deltaTime);

    }

    // Adjust the mouse speed
    if (::g_MouseIsInsideWindow)
    {
        const float MOUSE_WHEEL_SENSITIVITY = 0.1f;

        // Adjust the movement speed based on the wheel position
        ::g_FlyCamera->movementSpeed -= (::g_FlyCamera->getMouseWheel() * MOUSE_WHEEL_SENSITIVITY);

        // Clear the mouse wheel delta (or it will increase constantly)
        ::g_FlyCamera->clearMouseWheelValue();


        if (::g_FlyCamera->movementSpeed <= 0.0f)
        {
            ::g_FlyCamera->movementSpeed = 0.0f;
        }
    }
    return;
}

void ProcessAsyncKeyboard(GLFWwindow* window, float deltaTime)
{
    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_RELEASE && glfwGetKey(window, GLFW_KEY_LEFT_ALT) == GLFW_RELEASE
        && glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_RELEASE)
    {
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS && glfwGetKey(window, GLFW_KEY_S) != GLFW_PRESS)
        {
            g_FlyCamera->MoveForward_Z(flyCameraSpeed * deltaTime);
        }
        else if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS && glfwGetKey(window, GLFW_KEY_W) != GLFW_PRESS)
        {
            g_FlyCamera->MoveForward_Z(-flyCameraSpeed * deltaTime);
        }

        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS && glfwGetKey(window, GLFW_KEY_D) != GLFW_PRESS)
        {
            g_FlyCamera->MoveLeftRight_X(-flyCameraSpeed * deltaTime);
        }
        else if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS && glfwGetKey(window, GLFW_KEY_A) != GLFW_PRESS)
        {
            g_FlyCamera->MoveLeftRight_X(flyCameraSpeed * deltaTime);
        }

        if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS && glfwGetKey(window, GLFW_KEY_E) != GLFW_PRESS)
        {
            g_FlyCamera->MoveUpDown_Y(-flyCameraSpeed * deltaTime);
        }
        else if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS && glfwGetKey(window, GLFW_KEY_Q) != GLFW_PRESS)
        {
            g_FlyCamera->MoveUpDown_Y(flyCameraSpeed * deltaTime);
        }
    }
}

int main(void)
{
    GLFWwindow* window;

    g_FlyCamera = new cFlyCamera();
//    GLuint vertex_buffer = 0;     // ** NOW in VAO Manager **

//    GLuint vertex_shader;     // Now in the "Shader Manager"
//    GLuint fragment_shader;   // Now in the "Shader Manager"
    program = 0;     // 0 means "no shader program"

    GLint mvp_location = -1;        // Because glGetAttribLocation() returns -1 on error
//    GLint vpos_location = -1;       // Because glGetAttribLocation() returns -1 on error
//    GLint vcol_location = -1;       // Because glGetAttribLocation() returns -1 on error

    glfwSetErrorCallback(error_callback);


    if ( ! glfwInit() )
    {
        return -1;
        //exit(EXIT_FAILURE);
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

    window = glfwCreateWindow(1200, 640, "Ethan's Engine", NULL, NULL);

    if (!window)
    {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    glfwSetKeyCallback(window, key_callback);	// was “key_callback”
    // These are new:
    glfwSetCursorEnterCallback(window, GLFW_cursor_enter_callback);
    glfwSetScrollCallback(window, GLFW_scroll_callback);

    glfwMakeContextCurrent(window);
// Tiny change from the original documentation code
    gladLoadGLLoader( (GLADloadproc) glfwGetProcAddress);
//    gladLoadGL(glfwGetProcAddress);
    glfwSwapInterval(1);


    // void glGetIntegerv(GLenum pname, GLint * data);
    // See how many active uniform variables (registers) I can have
    GLint max_uniform_location = 0;
    GLint* p_max_uniform_location = NULL;
    p_max_uniform_location = &max_uniform_location;
    glGetIntegerv(GL_MAX_UNIFORM_LOCATIONS, p_max_uniform_location);

    cShaderManager::cShader vertShader;
    vertShader.fileName = "assets/shaders/vertShader_01.glsl";
        
    cShaderManager::cShader fragShader;
    fragShader.fileName = "assets/shaders/fragShader_01.glsl";

    if (gShaderManager.createProgramFromFile("Shader#1", vertShader, fragShader))
    {
        std::cout << "Shader compiled OK" << std::endl;
        // 
        // Set the "program" variable to the one the Shader Manager used...
        program = gShaderManager.getIDFromFriendlyName("Shader#1");
    }
    else
    {
        std::cout << "Error making shader program: " << std::endl;
        std::cout << gShaderManager.getLastError() << std::endl;
    }

    glUseProgram(program);

    mvp_location = glGetUniformLocation(program, "MVP");

    // Get "uniform locations" (aka the registers these are in)
    
    GLint matView_Location = glGetUniformLocation(program, "matView");
    GLint matProjection_Location = glGetUniformLocation(program, "matProjection");

    //OUTSIDE LIGHTS
    gTheLights.theLights[0].position = glm::vec4(-4.5f, 6.0f, -1.5f, 1.0f);
    gTheLights.theLights[0].diffuse = glm::vec4(1.0f, 0.6f, .05f, 1.0f);
    gTheLights.theLights[0].atten = glm::vec4(0.2f, 0.1f, 0.025f, 100000.0f);
    //gTheLights.theLights[0].direction = glm::vec4(0.0f, -1.0f, 1.0f, 1.0f);
    //gTheLights.theLights[0].specular = glm::vec4(1.0f, 1.0f, 1.0f, 50.0f);
    gTheLights.theLights[0].param1.x = 0;
    gTheLights.TurnOnLight(0);  // Or this!
    gTheLights.SetUpUniformLocations(program, 0);

    gTheLights.theLights[1].position = glm::vec4(3.25f, 5.5f, 8.5f, 1.0f);
    gTheLights.theLights[1].diffuse = glm::vec4(1.0f, 0.6f, .05f, 1.0f);
    gTheLights.theLights[1].atten = glm::vec4(0.2f, 0.1f, 0.025f, 100000.0f);
   // gTheLights.theLights[1].direction = glm::vec4(0.0f, -1.0f, 1.0f, 1.0f);
    //gTheLights.theLights[0].specular = glm::vec4(1.0f, 1.0f, 1.0f, 50.0f);
    gTheLights.theLights[1].param1.x = 0;
    gTheLights.TurnOnLight(1);  // Or this!
    gTheLights.SetUpUniformLocations(program, 1);

    gTheLights.theLights[2].position = glm::vec4(-2.5f, 5.5f, -8.5f, 1.0f);
    gTheLights.theLights[2].diffuse = glm::vec4(1.0f, 0.6f, .05f, 1.0f);
    gTheLights.theLights[2].atten = glm::vec4(0.2f, 0.1f, 0.025f, 100000.0f);
    //gTheLights.theLights[2].direction = glm::vec4(0.0f, -1.0f, 1.0f, 1.0f);
    //gTheLights.theLights[0].specular = glm::vec4(1.0f, 1.0f, 1.0f, 50.0f);
    gTheLights.theLights[2].param1.x = 0;
    gTheLights.TurnOnLight(2);  // Or this!
    gTheLights.SetUpUniformLocations(program, 2);

    gTheLights.theLights[3].position = glm::vec4(6.5f, 5.5f, -8.5f, 1.0f);
    gTheLights.theLights[3].diffuse = glm::vec4(1.0f, 0.6f, .05f, 1.0f);
    gTheLights.theLights[3].atten = glm::vec4(0.2f, 0.1f, 0.025f, 100000.0f);
    //gTheLights.theLights[3].direction = glm::vec4(0.0f, -1.0f, 1.0f, 1.0f);
    //gTheLights.theLights[0].specular = glm::vec4(1.0f, 1.0f, 1.0f, 50.0f);
    gTheLights.theLights[3].param1.x = 0;
    gTheLights.TurnOnLight(3);  // Or this!
    gTheLights.SetUpUniformLocations(program, 3);

    gTheLights.theLights[6].position = glm::vec4(10.5f, 5.5f, -0.25f, 1.0f);
    gTheLights.theLights[6].diffuse = glm::vec4(1.0f, 0.6f, .05f, 1.0f);
    gTheLights.theLights[6].atten = glm::vec4(0.2f, 0.1f, 0.025f, 100000.0f);
    //gTheLights.theLights[3].direction = glm::vec4(0.0f, -1.0f, 1.0f, 1.0f);
    //gTheLights.theLights[0].specular = glm::vec4(1.0f, 1.0f, 1.0f, 50.0f);
    gTheLights.theLights[6].param1.x = 0;
    gTheLights.TurnOnLight(6);  // Or this!
    gTheLights.SetUpUniformLocations(program, 6);

    gTheLights.theLights[7].position = glm::vec4(-10.5f, 6.5f, 4.0f, 1.0f);
    gTheLights.theLights[7].diffuse = glm::vec4(1.0f, 0.6f, .05f, 1.0f);
    gTheLights.theLights[7].atten = glm::vec4(0.2f, 0.1f, 0.025f, 100000.0f);
    //gTheLights.theLights[3].direction = glm::vec4(0.0f, -1.0f, 1.0f, 1.0f);
    //gTheLights.theLights[0].specular = glm::vec4(1.0f, 1.0f, 1.0f, 50.0f);
    gTheLights.theLights[7].param1.x = 0;
    gTheLights.TurnOnLight(7);  // Or this!
    gTheLights.SetUpUniformLocations(program, 7);

    gTheLights.theLights[9].position = glm::vec4(4.25f, 6.0f, 26.5f, 1.0f);
    gTheLights.theLights[9].diffuse = glm::vec4(1.0f, 0.6f, .05f, 1.0f);
    gTheLights.theLights[9].atten = glm::vec4(0.2f, 0.1f, 0.025f, 100000.0f);
    //gTheLights.theLights[0].direction = glm::vec4(0.0f, -1.0f, 1.0f, 1.0f);
    //gTheLights.theLights[0].specular = glm::vec4(1.0f, 1.0f, 1.0f, 50.0f);
    gTheLights.theLights[9].param1.x = 0;
    gTheLights.TurnOnLight(9);  // Or this!
    gTheLights.SetUpUniformLocations(program, 9);


    //INSIDE LIGHTS
    gTheLights.theLights[4].position = glm::vec4(1.5f, 7.5f, -0.25f, 1.0f);
    gTheLights.theLights[4].diffuse = glm::vec4(0.76f, 0.9f, 1.0f, 1.0f);
    gTheLights.theLights[4].atten = glm::vec4(0.2f, 0.1f, 0.005f, 100000.0f);
    gTheLights.theLights[4].direction = glm::vec4(0.0f, -1.0f, 0.0f, 1.0f);
    //gTheLights.theLights[0].specular = glm::vec4(1.0f, 1.0f, 1.0f, 50.0f);
    gTheLights.theLights[4].param1.x = 1;
    gTheLights.theLights[4].param1.y = 20.0f;
    gTheLights.theLights[4].param1.z = 25.0f;
    gTheLights.TurnOnLight(4);  // Or this!
    gTheLights.SetUpUniformLocations(program, 4);

    gTheLights.theLights[5].position = glm::vec4(1.5f, 7.5f, 3.5f, 1.0f);
    gTheLights.theLights[5].diffuse = glm::vec4(0.76f, 0.9f, 1.0f, 1.0f);
    gTheLights.theLights[5].atten = glm::vec4(0.2f, 0.1f, 0.005f, 100000.0f);
    gTheLights.theLights[5].direction = glm::vec4(0.0f, -1.0f, 0.0f, 1.0f);
    //gTheLights.theLights[0].specular = glm::vec4(1.0f, 1.0f, 1.0f, 50.0f);
    gTheLights.theLights[5].param1.x = 1;
    gTheLights.theLights[5].param1.y = 20.0f;
    gTheLights.theLights[5].param1.z = 25.0f;
    gTheLights.TurnOnLight(5);  // Or this!
    gTheLights.SetUpUniformLocations(program, 5);

    //SUN
    gTheLights.theLights[8].position = glm::vec4(0.f, 0.f, 0.f, 1.0f);
    gTheLights.theLights[8].diffuse = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
    gTheLights.theLights[8].atten = glm::vec4(0.2f, 0.1f, 0.005f, 100000.0f);
    gTheLights.theLights[8].direction = glm::vec4(0.0f, -1.0f, -.2f, 1.0f);
    //gTheLights.theLights[0].specular = glm::vec4(1.0f, 1.0f, 1.0f, 50.0f);
    gTheLights.theLights[8].param1.x = 2;
    gTheLights.TurnOffLight(8);  // Or this!
    gTheLights.SetUpUniformLocations(program, 8);

    gTheLights.theLights[12].position = glm::vec4(0.f, 0.f, 0.f, 1.0f);
    gTheLights.theLights[12].diffuse = glm::vec4(.5f, 0.5f, .5f, 1.0f);
    gTheLights.theLights[12].atten = glm::vec4(0.2f, 0.1f, 0.005f, 100000.0f);
    gTheLights.theLights[12].direction = glm::vec4(0.0f, .9f, .4f, 1.0f);
    //gTheLights.theLights[0].specular = glm::vec4(1.0f, 1.0f, 1.0f, 50.0f);
    gTheLights.theLights[12].param1.x = 2;
    gTheLights.TurnOffLight(12);  // Or this!
    gTheLights.SetUpUniformLocations(program, 12);

    sceneLoader = cSceneLoader::GetSceneLoaderInstance();

    //Gets the path of the texture manager to the texture folder
    g_textureManager.SetBasePath("assets/textures");

    std::string errorString;
    if (!g_textureManager.CreateCubeTextureFromBMPFiles("TropicalSunnyDay",
        "TropicalSunnyDayRight2048.bmp",
        "TropicalSunnyDayLeft2048.bmp",
        "TropicalSunnyDayUp2048.bmp",
        "TropicalSunnyDayDown2048.bmp",
        "TropicalSunnyDayFront2048.bmp",
        "TropicalSunnyDayBack2048.bmp",
        //Is seamless
        true, errorString))
    {
        std::cout << errorString << std::endl;
    }

    if (!g_textureManager.CreateCubeTextureFromBMPFiles("NightSky",
        "SpaceBox_right1_posX.bmp",
        "SpaceBox_left2_negX.bmp",
        "SpaceBox_top3_posY.bmp",
        "SpaceBox_bottom4_negY.bmp",
        "SpaceBox_front5_posZ.bmp",
        "SpaceBox_back6_negZ.bmp",
        //Is seamless
        true, errorString))
    {
        std::cout << errorString << std::endl;
    }

    g_textureManager.Create2DTextureFromBMPFile("smoke.bmp", true);

    //Loads the scene and the textures used in the scene
    std::cout << "Loading scene " << sceneName << "...." << std::endl;
    if (sceneLoader->LoadScene(sceneName, &g_textureManager, &g_entityManager))
    {
        std::cout << "Loaded scene: " << sceneName << std::endl << std::endl;
        std::cout << "Loading scene into VAO manager...." << std::endl;
        if (sceneLoader->LoadIntoVAO(&gVAOManager, program))
        {
            std::cout << "Loaded scene into VAO manager" << std::endl << std::endl;

            g_FlyCamera->setEye(sceneLoader->GetCameraStartingPosition());
        }
        else
        {
            std::cout << "ERROR: Issue loading models into vao" << std::endl;
        }
    }
    else
    {
        std::cout << "ERROR: Issue loading scene" << std::endl;
        return -1;
    }

    std::vector<cEntity*> entities = g_entityManager.GetEntities();

    //gets the sky box model so we can change the skybox texture
    g_skyBox = *std::find_if(entities.begin(), entities.end(), [](cEntity* mesh) { return mesh->GetComponent<cMeshRenderer>()->friendlyName == "Sky"; });

    std::map<std::string, GLint> uniformLocations;

    uniformLocations.insert(std::pair<std::string, GLint>("matModel", glGetUniformLocation(program, "matModel")));
    uniformLocations.insert(std::pair<std::string, GLint>("matModelInverseTranspose", glGetUniformLocation(program, "matModelInverseTranspose")));
    // Copy the whole object colour information to the sahder               

            // This is used for wireframe or whole object colour. 
            // If bUseDebugColour is TRUE, then the fragment colour is "objectDebugColour".
    uniformLocations.insert(std::pair<std::string, GLint>("bUseDebugColour", glGetUniformLocation(program, "bUseDebugColour")));
    uniformLocations.insert(std::pair<std::string, GLint>("objectDebugColour", glGetUniformLocation(program, "objectDebugColour")));

    // If true, then the lighting contribution is NOT used. 
    // This is useful for wireframe object
    uniformLocations.insert(std::pair<std::string, GLint>("bDontLightObject", glGetUniformLocation(program, "bDontLightObject")));

    // The "whole object" colour (diffuse and specular)
    uniformLocations.insert(std::pair<std::string, GLint>("wholeObjectDiffuseColour", glGetUniformLocation(program, "wholeObjectDiffuseColour")));
    uniformLocations.insert(std::pair<std::string, GLint>("bUseWholeObjectDiffuseColour", glGetUniformLocation(program, "bUseWholeObjectDiffuseColour")));
    uniformLocations.insert(std::pair<std::string, GLint>("wholeObjectSpecularColour", glGetUniformLocation(program, "wholeObjectSpecularColour")));
    uniformLocations.insert(std::pair<std::string, GLint>("bUseSpecular", glGetUniformLocation(program, "bUseSpecular")));
    uniformLocations.insert(std::pair<std::string, GLint>("eyeLocation", glGetUniformLocation(program, "eyeLocation")));

    uniformLocations.insert(std::pair<std::string, GLint>("texture_00", glGetUniformLocation(program, "texture_00")));
    uniformLocations.insert(std::pair<std::string, GLint>("texture_01", glGetUniformLocation(program, "texture_01")));
    uniformLocations.insert(std::pair<std::string, GLint>("texture_02", glGetUniformLocation(program, "texture_02")));
    uniformLocations.insert(std::pair<std::string, GLint>("texture_03", glGetUniformLocation(program, "texture_03")));

    uniformLocations.insert(std::pair<std::string, GLint>("texLightpassColorBuf", glGetUniformLocation(program, "texLightpassColorBuf")));

    uniformLocations.insert(std::pair<std::string, GLint>("texture_MatColor", glGetUniformLocation(program, "texture_MatColor")));
    uniformLocations.insert(std::pair<std::string, GLint>("texture_Normal", glGetUniformLocation(program, "texture_Normal")));
    uniformLocations.insert(std::pair<std::string, GLint>("texture_WorldPos", glGetUniformLocation(program, "texture_WorldPos")));
    uniformLocations.insert(std::pair<std::string, GLint>("texture_Specular", glGetUniformLocation(program, "texture_Specular")));

    uniformLocations.insert(std::pair<std::string, GLint>("textureRatios", glGetUniformLocation(program, "textureRatios")));

    uniformLocations.insert(std::pair<std::string, GLint>("bUseAlphaMask", glGetUniformLocation(program, "bUseAlphaMask")));
    uniformLocations.insert(std::pair<std::string, GLint>("alphaMask", glGetUniformLocation(program, "alphaMask")));

    uniformLocations.insert(std::pair<std::string, GLint>("bUseNormalMap", glGetUniformLocation(program, "bUseNormalMap")));
    uniformLocations.insert(std::pair<std::string, GLint>("normalMap", glGetUniformLocation(program, "normalMap")));
    uniformLocations.insert(std::pair<std::string, GLint>("normalOffset", glGetUniformLocation(program, "normalOffset")));

    uniformLocations.insert(std::pair<std::string, GLint>("bUseHeightMap", glGetUniformLocation(program, "bUseHeightMap")));
    uniformLocations.insert(std::pair<std::string, GLint>("heightMap", glGetUniformLocation(program, "heightMap")));

    uniformLocations.insert(std::pair<std::string, GLint>("bUseSkybox", glGetUniformLocation(program, "bUseSkybox")));
    uniformLocations.insert(std::pair<std::string, GLint>("skyBox", glGetUniformLocation(program, "skyBox")));

    uniformLocations.insert(std::pair<std::string, GLint>("bDebugMode", glGetUniformLocation(program, "bDebugMode")));
    uniformLocations.insert(std::pair<std::string, GLint>("bDebugShowLighting", glGetUniformLocation(program, "bDebugShowLighting")));
    uniformLocations.insert(std::pair<std::string, GLint>("bDebugShowNormals", glGetUniformLocation(program, "bDebugShowNormals")));

    uniformLocations.insert(std::pair<std::string, GLint>("bIsImposter", glGetUniformLocation(program, "bIsImposter")));

    uniformLocations.insert(std::pair<std::string, GLint>("passNumber", glGetUniformLocation(program, "passNumber")));
    uniformLocations.insert(std::pair<std::string, GLint>("screenWidthHeight", glGetUniformLocation(program, "screenWidthHeight")));

    /*sModelDrawInfo debugSphere;
    if (!gVAOManager.LoadModelIntoVAO("ISO_Shphere_flat_3div_xyz_n_rgba_uv.ply", debugSphere, program))
    {
        std::cout << "Couldn't load debug sphere" << std::endl;
    }*/

    g_DebugSphere = g_entityManager.CreateEntity(false);

    cMeshRenderer* debugMesh = new cMeshRenderer();
    debugMesh->meshName = "Beachball.ply";
    debugMesh->bIsWireframe = true;
    g_DebugSphere->AddComponent<cMeshRenderer>(debugMesh);

    cMeshRenderer* cloudOne = new cMeshRenderer();
    cloudOne->meshName = "cloud1.ply";
    cloudOne->bUseWholeObjectDiffuseColour = true;
    cloudOne->wholeObjectDiffuseRGBA = glm::vec4(0.9f, 0.9f, 1.0f, 0.10f);

    cEntity* cloudOneEntity = g_entityManager.CreateEntity();
    cloudOneEntity->GetComponent<cTransform>()->position = glm::vec3(0.0f, 20.0f, -18.0f);

    cloudOneEntity->AddComponent<cMeshRenderer>(cloudOne);
    clouds.push_back(cloudOneEntity);

    cMeshRenderer* cloudTwo = new cMeshRenderer();
    cloudTwo->meshName = "cloud2.ply";  
    cloudTwo->bUseWholeObjectDiffuseColour = true;
    cloudTwo->wholeObjectDiffuseRGBA = glm::vec4(1.0f, 1.0f, 1.0f, 0.92f);
    

    cEntity* cloudTwoEntity = g_entityManager.CreateEntity();

    cTransform* cloudTwoTransform = cloudTwoEntity->GetComponent<cTransform>();
    cloudTwoTransform->position = glm::vec3(-15.0f, 24.0f, 0.0f);
    cloudTwoTransform->SetRotation(glm::vec3(0.0f, glm::radians(90.f), 0.0f));
    cloudTwoTransform->scale = glm::vec3(0.65f);

    cloudTwoEntity->AddComponent<cMeshRenderer>(cloudTwo);
    clouds.push_back(cloudTwoEntity);

    cMeshRenderer* cloudThree = new cMeshRenderer();
    cloudThree->meshName = "cloud1.ply";
    cloudThree->bUseWholeObjectDiffuseColour = true;
    cloudThree->wholeObjectDiffuseRGBA = glm::vec4(0.9f, 0.9f, 1.0f, 0.50f);

    cEntity* cloudThreeEntity = g_entityManager.CreateEntity();

    cTransform* cloudThreeTransform = cloudTwoEntity->GetComponent<cTransform>();
    cloudThreeTransform->position = glm::vec3(20.0f, 19.0f, -28.0f);
    cloudThreeTransform->scale = glm::vec3(0.78f);

    cloudThreeEntity->AddComponent<cMeshRenderer>(cloudThree);
    clouds.push_back(cloudThreeEntity);

    const float MAX_DELTA_TIME = 0.1f;	// 100 ms
    float previousTime = (float)glfwGetTime();

    cWorld* world = new cWorld();

    cEmitters emitters;
    emitters.SetWorld(world);

    cEmitter emitOne = cEmitter(0.3f, glm::vec3(-4.5f, 6.0f, -1.5f));
    emitters.AddEmitter(&emitOne);

    cEmitter emitTwo = cEmitter(0.3f, glm::vec3(3.5f, 6.f, 8.f));
    emitters.AddEmitter(&emitTwo);

    cEmitter emitThree = cEmitter(0.3f, glm::vec3(0.5f, 6.f, -5.5f));
    emitters.AddEmitter(&emitThree);

    cEmitter emitFour = cEmitter(0.3f, glm::vec3(4.4f, 6.f, -5.5f));
    emitters.AddEmitter(&emitFour);

    cEmitter emitFive = cEmitter(0.3f, glm::vec3(8.3f, 6.f, -5.5f));
    emitters.AddEmitter(&emitFive);

    cEmitter emitSix = cEmitter(0.3f, glm::vec3(-9.25f, 6.f, 7.5f));
    emitters.AddEmitter(&emitSix);

    cForceGenerator gravity = cForceGenerator(glm::vec3(0.0f, 0.05f, 0.0f));
    emitters.SetForce(&gravity);

    float ratio;
    int width, height;

    glfwGetFramebufferSize(window, &width, &height);
    ratio = width / (float)height;

    int screenPixelDensity = 2000;

    g_fbo = new cFBO();
    if (!g_fbo->init((int)screenPixelDensity * ratio, screenPixelDensity, errorString))
    {
        std::cout << "Error in FBO" << std::endl;
    }

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    const GLint RENDER_PASS_0_G_BUFFER = 0;
    const GLint RENDER_PASS_1_LIGHTING = 1;
    const GLint RENDER_PASS_2_EFFECTS = 2;

    while (!glfwWindowShouldClose(window))
    {
        glUniform1ui(uniformLocations["passNumber"], RENDER_PASS_0_G_BUFFER);

        float currentTime = (float)glfwGetTime();
        float deltaTime = currentTime - previousTime;
        deltaTime = (deltaTime > MAX_DELTA_TIME ? MAX_DELTA_TIME : deltaTime);
        previousTime = currentTime;

        glm::mat4 matModel;    // used to be "m"; Sometimes it's called "world"
        glm::mat4 p;
        glm::mat4 v;

        glBindFramebuffer(GL_FRAMEBUFFER, g_fbo->ID);

        glViewport(0, 0, g_fbo->width, g_fbo->height);
        ratio = g_fbo->width / (float)g_fbo->height;
        // Turn on the depth buffer
        glEnable(GL_DEPTH);         // Turns on the depth buffer
        glEnable(GL_DEPTH_TEST);    // Check if the pixel is already closer

        //glEnable(GL_CULL_FACE);
       // glCullFace(GL_BACK);

        //glfwGetFramebufferSize(window, &width, &height);
        
        g_fbo->clearBuffers(true, true);


        //glViewport(0, 0, width, height);

        //glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
       // glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // *******************************************************
        // Screen is cleared and we are ready to draw the scene...
        // *******************************************************

        // Copy the light information into the shader to draw the scene
        gTheLights.CopyLightInfoToShader();

        // Place the "debug sphere" at the same location as the selected light (again)
        // HACK: Debug sphere is 5th item added
        p = glm::perspective(glm::radians(75.0f),
            ratio,
            0.1f,
            1000.0f);     

        v = glm::mat4(1.0f);

        //glm::vec3 cameraEye = glm::vec3(0.0, 0.0, -4.0f);
        glm::vec3 cameraTarget = glm::vec3(0.0f, 0.0f, 0.0f);
        glm::vec3 upVector = glm::vec3(0.0f, 1.0f, 0.0f);

        glm::vec3 cameraEye = g_FlyCamera->getEye();

        v = glm::lookAt(cameraEye,     // "eye"
            g_FlyCamera->getAtInWorldSpace(),  // "at"
            g_FlyCamera->getUpVector());

        glUniform4f(uniformLocations["eyeLocation"], cameraEye.x, cameraEye.y, cameraEye.z, 1.0f);

        glUniformMatrix4fv(matView_Location, 1, GL_FALSE, glm::value_ptr(v));
        glUniformMatrix4fv(matProjection_Location, 1, GL_FALSE, glm::value_ptr(p));

        //Sort based on transparency
        std::vector<cEntity*> transparentMeshes;
        std::vector<cEntity*> opaqueMeshes;

        std::vector<cEntity*> entityVector = g_entityManager.GetEntities();
        for (unsigned int index = 0; index != entityVector.size(); index++)
        {
            cMeshRenderer* curMesh = entityVector.at(index)->GetComponent<cMeshRenderer>();

            if (curMesh->wholeObjectDiffuseRGBA.a < 1.0f)
            {
                transparentMeshes.push_back(entityVector.at(index));
            }
            else
            {
                opaqueMeshes.push_back(entityVector.at(index));
            }
        }

        std::vector<cParticle*>* particleList = world->GetParticleList();
        //Add particles to render list
        for (unsigned int index = 0; index != particleList->size(); index++)
        {
            cEntity* curEntity = particleList->at(index)->entity;
            cMeshRenderer* curMesh = curEntity->GetComponent<cMeshRenderer>();

            if (curMesh != nullptr)
            {
                if ((curMesh->wholeObjectDiffuseRGBA.a < 1.0f || curMesh->bUseAlphaMask || curMesh->bIsImposter))
                {
                    transparentMeshes.push_back(curEntity);
                }
                else
                {
                    opaqueMeshes.push_back(curEntity);
                }
            }
        }

        //Sort transparent objects.
        std::sort(transparentMeshes.begin(), transparentMeshes.end(), DistanceToCameraPredicate);

        //Draw non transparent objects
        for (unsigned int index = 0; index != opaqueMeshes.size(); index++)
        {         
            cEntity* curEntity = opaqueMeshes[index];
            matModel = glm::mat4(1.0f);  // "Identity" ("do nothing", like x1)
            //mat4x4_identity(m);

            if (curEntity->GetComponent<cMeshRenderer>()->friendlyName == "Sky")
            {
                curEntity->GetComponent<cTransform>()->position = g_FlyCamera->getEye();
            }

            DrawObject(curEntity, matModel, program, &gVAOManager, g_textureManager, uniformLocations, g_FlyCamera->getEye());
        }//for (unsigned int index

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        //Draw transparent objects
        for (unsigned int index = 0; index != transparentMeshes.size(); index++)
        {
            cEntity* curEntity = transparentMeshes[index];
            cMeshRenderer* curMesh = curEntity->GetComponent<cMeshRenderer>();
            matModel = glm::mat4(1.0f);  // "Identity" ("do nothing", like x1)
            //mat4x4_identity(m);

            //Scroll pool normal effect
            if (curMesh->friendlyName == "poolwater")
            {
                curMesh->normalOffset.x += (float)deltaTime / 50.0f;
                curMesh->normalOffset.y += (float)deltaTime / 75.0f;
            }

            DrawObject(curEntity, matModel, program, &gVAOManager, g_textureManager, uniformLocations, g_FlyCamera->getEye());
        }//for (unsigned int index

        //Render debug sphere
        if (isDebugMode)
        {
            glUniform1f(uniformLocations["bDebugMode"], (float)GL_TRUE);

            glUniform1f(uniformLocations["bDebugShowLighting"], (float)debugShowLighting);
            glUniform1f(uniformLocations["bDebugShowNormals"], (float)debugShowNormals);

            cMeshRenderer* debugSphereMesh = g_DebugSphere->GetComponent<cMeshRenderer>();
            cTransform* debugSphereTransform = g_DebugSphere->GetComponent<cTransform>();

            debugSphereTransform->position = gTheLights.theLights[g_selectedLight].position;
            debugSphereTransform->scale = glm::vec3(0.01f);
            debugSphereMesh->bDontLight = true;
            debugSphereMesh->bUseObjectDebugColour = true;
            debugSphereMesh->objectDebugColourRGBA = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);

            glm::mat4 matModelDS = glm::mat4(1.0f);
            DrawObject(g_DebugSphere, matModelDS, program, &gVAOManager, g_textureManager, uniformLocations, g_FlyCamera->getEye());
        }
        else
        {
            glUniform1f(uniformLocations["bDebugMode"], (float)GL_FALSE);
        }

        //End of first pass

        glm::vec3 fullscreenPos = glm::vec3(0.f, 0.f, -1.f);

        cMeshRenderer* fullscreenMesh = new cMeshRenderer();
        fullscreenMesh->meshName = "fullscreenquad.ply";
        fullscreenMesh->friendlyName = "Fullscreen Quad";

        fullscreenMesh->bDontLight = true;
        fullscreenMesh->bUseWholeObjectDiffuseColour = true;
        fullscreenMesh->wholeObjectDiffuseRGBA = glm::vec4(1.0f);
        

        cEntity* fullscreenEntity = g_entityManager.CreateEntity(false);
        fullscreenEntity->AddComponent<cMeshRenderer>(fullscreenMesh);

        cTransform* fullscreenTransform = fullscreenEntity->GetComponent<cTransform>();
        fullscreenTransform->scale = glm::vec3(10.0f);
        fullscreenTransform->Rotate(glm::vec3(0.0f, glm::radians(180.0f), 0.0f));

        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        //glfwGetFramebufferSize(window, &width, &height);
       // ratio = width / (float)height;
         //glViewport(0, 0, width, height);

        glViewport(0, 0, g_fbo->width, g_fbo->height);
        ratio = g_fbo->width / (float)g_fbo->height;

        //p = glm::perspective(glm::radians(75.0f),
        //    ratio,
        //    0.1f,
        //    1000.0f);

        v = glm::lookAt(fullscreenPos,     // "eye"
            fullscreenPos + glm::vec3(0.f, 0.f, 1.0f) ,  // "at"
            g_FlyCamera->getUpVector());

        p = glm::ortho(0.0f, 1.0f/(float)width, 0.0f, 1.0f / (float)height, 0.01f, 1000.0f);

        glUniformMatrix4fv(matView_Location, 1, GL_FALSE, glm::value_ptr(v));
        glUniformMatrix4fv(matProjection_Location, 1, GL_FALSE, glm::value_ptr(p));

       

        glUniform2f(uniformLocations["screenWidthHeight"], width, height);
        glUniform1ui(uniformLocations["passNumber"], RENDER_PASS_1_LIGHTING);



        //Uploading textures to gpu
        GLint textureMatId = g_fbo->vertexMatColour_1_ID;
        if (textureMatId != 0)
        {
            GLint unit = 0;
            glActiveTexture(unit + GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, textureMatId);
            glUniform1i(uniformLocations["texture_MatColor"], unit);
        }

        GLint textureNormalId = g_fbo->vertexNormal_2_ID;
        if (textureNormalId != 0)
        {
            GLint unit = 1;
            glActiveTexture(unit + GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, textureNormalId);
            glUniform1i(uniformLocations["texture_Normal"], unit);
        }

        GLint textureWorldId = g_fbo->vertexWorldPos_3_ID;
        if (textureWorldId != 0)
        {
            GLint unit = 2;
            glActiveTexture(unit + GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, textureWorldId);
            glUniform1i(uniformLocations["texture_WorldPos"], unit);
        }

        GLint textureSpecularId = g_fbo->vertexSpecular_4_ID;
        if (textureSpecularId != 0)
        {
            GLint unit = 3;
            glActiveTexture(unit + GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, textureSpecularId);
            glUniform1i(uniformLocations["texture_Specular"], unit);
        }

        DrawObject(fullscreenEntity, glm::mat4(1.0f), program, &gVAOManager, g_textureManager, uniformLocations, fullscreenPos);

        glUniform1ui(uniformLocations["passNumber"], RENDER_PASS_2_EFFECTS);

        GLint textureId = g_fbo->colourTexture_0_ID;
        if (textureId != 0)
        {
            GLint unit = 0;
            glActiveTexture(unit + GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, textureId);
            glUniform1i(uniformLocations["texLightpassColorBuf"], unit);
        }

        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);

        

        g_entityManager.DeleteEntity(fullscreenEntity);
        // "Present" what we've drawn.
        glfwSwapBuffers(window);
        glfwPollEvents();

        ProcessAsyncMouse(window, (float)deltaTime);
        ProcessAsyncKeyboard(window, (float)deltaTime);

        //Flicker lights
        gTheLights.theLights[0].atten.x = (rand() % 200 + 100) / 1000.0f;
        gTheLights.theLights[1].atten.x = (rand() % 200 + 100) / 1000.0f;
        gTheLights.theLights[2].atten.x = (rand() % 200 + 100) / 1000.0f;
        gTheLights.theLights[3].atten.x = (rand() % 200 + 100) / 1000.0f;
        gTheLights.theLights[6].atten.x = (rand() % 200 + 100) / 1000.0f;
        gTheLights.theLights[7].atten.x = (rand() % 200 + 100) / 1000.0f;

        //Move clouds
        for (cEntity* cloud : clouds)
        {
            cTransform* transform = cloud->GetComponent<cTransform>();

            transform->position.z +=  1.0f * (float)deltaTime;

            if (transform->position.z > 30.0f)
                transform->position.z = -30.0f;
        }

        if (preWarm)
        {
            for (int i = 0; i < 100; i++)
            {
                world->EulerTimeStep(0.3f);
                emitters.Integrate(0.3f);
            }

            preWarm = false;
        }
        else
        {
            world->EulerTimeStep(deltaTime);
            emitters.Integrate(deltaTime);
        }
       
        g_FlyCamera->Update(deltaTime);   
    }

    delete g_FlyCamera;
    delete world;
    g_entityManager.DeleteEntity(g_DebugSphere);

    if (!g_fbo->shutdown())
    {
        std::cout << "Error shutting down fbo" << std::endl;
    }

    glfwDestroyWindow(window);

    glfwTerminate();
    exit(EXIT_SUCCESS);
}

