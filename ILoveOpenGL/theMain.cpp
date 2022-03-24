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

#include "cEntity.h"
#include "cBasicTextureManager/cBasicTextureManager.h"
#include <algorithm>
#include "Physics/cWorld.h"
#include "cEmitters.h"
#include "Physics/cForceGenerator.h"

#include "cFBO/cFBO.h"
#include "cMeshRenderer.h"
#include "cEntityManager.h"
#include "cTransform.h"
#include "cTextureViewer.h"
#include "cPingPongFBOs.h"

enum class Transform
{
    Translate,
    Rotate,
    Scale
};

struct PostProcessingInfo
{
    float gamma = 1.85f;
    float exposure = 1.0f;

    bool useExposureToneMapping = false;

    float bloomThreshhold = 1.0f;
    float bloomSize = 1.0f;
    unsigned int bloomIterationAmount = 12;
};

GLuint program;
cFBO* g_fbo;

// Global things are here:

glm::vec3 cameraEye = glm::vec3(0);

glm::vec3 cameraDir = glm::vec3(0.0f, 0.0f, 1.0f);
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
glm::vec3 cameraRight = glm::vec3(0);

bool firstMouse = true;
float lastX;
float lastY;

float yaw = 90.0f;
float pitch = 0.0f;

float flyCameraSpeed = 5.0f;

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

bool isDebugMode = false;
bool debugShowLighting = true;
bool debugShowNormals = false;

int showTextureIndex = 0;

cEntity* g_skyBox;

bool showDebugGui = true;
ImGuiIO* io = nullptr;
size_t selectedEntityDebug = 0;
size_t selectedLightDebug = 0;

float fov = 70.0f;

PostProcessingInfo postProcessing;
cPingPongFBOs* pingPongFBO;

bool g_MouseIsInsideWindow = false;

//Method in DrawObjectFunction
void extern DrawObject(cEntity* curEntity, glm::mat4 matModel, GLint program, cVAOManager* VAOManager,
    cBasicTextureManager textureManager, std::map<std::string, GLint> uniformLocations, glm::vec3 eyeLocation);

void Draw(std::vector<cEntity*> opaqueMeshes, std::vector<cEntity*> transparentMeshes, std::map<std::string, int> uniformLocations, float deltaTime);
void DrawGUI(float dt);

static void error_callback(int error, const char* description)
{
    fprintf(stderr, "Error: %s\n", description);
}

static void GLFW_cursor_enter_callback(GLFWwindow* window, int entered)
{
    if (entered)
    {
        // std::cout << "Mouse cursor is over the window" << std::endl;
        g_MouseIsInsideWindow = true;
    }
    else
    {
        //std::cout << "Mouse cursor is no longer over the window" << std::endl;
        g_MouseIsInsideWindow = false;
    }
    return;
}

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (!io->WantCaptureKeyboard)
    {
        if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        {
            glfwSetWindowShouldClose(window, GLFW_TRUE);
        }
        else if (key == GLFW_KEY_GRAVE_ACCENT && action == GLFW_PRESS)
        {
            showDebugGui = !showDebugGui;
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
        else if (key == GLFW_KEY_SPACE && action == GLFW_PRESS)
        {
            //if (sceneLoader->SaveScene(sceneName, g_FlyCamera->getEye(), &g_entityManager))
            //{
            //    std::cout << "Saved scene: " << sceneName << std::endl;
            //}
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
    }
    return;
}

bool DistanceToCameraPredicate(cEntity* a, cEntity* b)
{
    if (glm::distance(a->GetComponent<cTransform>()->position, cameraEye) > glm::distance(b->GetComponent<cTransform>()->position, cameraEye))
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
    if (!io->WantCaptureMouse)
    {
        double x, y;
        glfwGetCursorPos(window, &x, &y);

        if (firstMouse)
        {
            lastX = (float)x;
            lastY = (float)y;

            firstMouse = false;
        }

        double xDelta = x - lastX;
        double yDelta = lastY - y;
        lastX = (float)x;
        lastY = (float)y;


        if ((glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
            && g_MouseIsInsideWindow)
        {
            const double MOUSE_SENSITIVITY = .1f;

            xDelta *= MOUSE_SENSITIVITY;
            yDelta *= MOUSE_SENSITIVITY;

            yaw += (float)xDelta;
            pitch += (float)yDelta;

            if (pitch > 89.0f)
                pitch = 89.0f;
            if (pitch < -89.0f)
                pitch = -89.0f;

            glm::vec3 direction;
            direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
            direction.y = sin(glm::radians(pitch));
            direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
            cameraDir = glm::normalize(direction);

            cameraRight = glm::normalize(glm::cross(glm::vec3(0.0f, 1.0f, 0.0f), cameraDir));
            cameraUp = glm::cross(cameraDir, cameraRight);
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
            cameraEye += cameraDir * flyCameraSpeed * deltaTime;
        }
        else if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS && glfwGetKey(window, GLFW_KEY_W) != GLFW_PRESS)
        {
            cameraEye -= cameraDir * flyCameraSpeed * deltaTime;
        }

        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS && glfwGetKey(window, GLFW_KEY_D) != GLFW_PRESS)
        {
            cameraEye += cameraRight * flyCameraSpeed * deltaTime;
        }
        else if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS && glfwGetKey(window, GLFW_KEY_A) != GLFW_PRESS)
        {
            cameraEye -= cameraRight * flyCameraSpeed * deltaTime;
        }

        if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS && glfwGetKey(window, GLFW_KEY_E) != GLFW_PRESS)
        {
            cameraEye -= glm::vec3(0.0f, 1.0f, 0.0f) * flyCameraSpeed * deltaTime;
        }
        else if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS && glfwGetKey(window, GLFW_KEY_Q) != GLFW_PRESS)
        {
            cameraEye += glm::vec3(0.0f, 1.0f, 0.0f) * flyCameraSpeed * deltaTime;
        }
    }
}

int main(void)
{
    GLFWwindow* window;
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

    window = glfwCreateWindow(1800, 1000, "Ethan's Engine", NULL, NULL);

    if (!window)
    {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    glfwSetKeyCallback(window, key_callback);	// was “key_callback”
    // These are new:
    glfwSetCursorEnterCallback(window, GLFW_cursor_enter_callback);
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
    glGetIntegerv(GL_MAX_COLOR_ATTACHMENTS, p_max_uniform_location);

    cShaderManager::cShader vertShader;
    vertShader.fileName = "assets/shaders/vertShader_01.glsl";
        
    cShaderManager::cShader fragShader;
    fragShader.fileName = "assets/shaders/fragShader_01.glsl";

    cShaderManager::cShader pingPongFragShader;
    pingPongFragShader.fileName = "assets/shaders/pingpongShader.glsl";

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

    if (gShaderManager.createProgramFromFile("PingPong", vertShader, pingPongFragShader))
    {
        std::cout << "Shader compiled OK" << std::endl;
        // 
        // Set the "program" variable to the one the Shader Manager used...
       // program = gShaderManager.getIDFromFriendlyName("Shader#1");
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
    gTheLights.theLights[0].name = "Outside light";
    gTheLights.theLights[0].position = glm::vec4(-4.5f, 6.0f, -1.5f, 1.0f);
    gTheLights.theLights[0].diffuse = glm::vec4(1.0f, 0.6f, .05f, 1.0f);
    gTheLights.theLights[0].atten = glm::vec4(0.2f, 0.1f, 0.025f, 100000.0f);
    //gTheLights.theLights[0].direction = glm::vec4(0.0f, -1.0f, 1.0f, 1.0f);
    //gTheLights.theLights[0].specular = glm::vec4(1.0f, 1.0f, 1.0f, 50.0f);
    gTheLights.theLights[0].param1.x = 0;
    gTheLights.TurnOnLight(0);  // Or this!
    gTheLights.SetUpUniformLocations(program, 0);

    gTheLights.theLights[1].name = "Outside light";
    gTheLights.theLights[1].position = glm::vec4(3.25f, 5.5f, 8.5f, 1.0f);
    gTheLights.theLights[1].diffuse = glm::vec4(1.0f, 0.6f, .05f, 1.0f);
    gTheLights.theLights[1].atten = glm::vec4(0.2f, 0.1f, 0.025f, 100000.0f);
    // gTheLights.theLights[1].direction = glm::vec4(0.0f, -1.0f, 1.0f, 1.0f);
     //gTheLights.theLights[0].specular = glm::vec4(1.0f, 1.0f, 1.0f, 50.0f);
    gTheLights.theLights[1].param1.x = 0;
    gTheLights.TurnOnLight(1);  // Or this!
    gTheLights.SetUpUniformLocations(program, 1);

    gTheLights.theLights[2].name = "Outside light";
    gTheLights.theLights[2].position = glm::vec4(-2.5f, 5.5f, -8.5f, 1.0f);
    gTheLights.theLights[2].diffuse = glm::vec4(1.0f, 0.6f, .05f, 1.0f);
    gTheLights.theLights[2].atten = glm::vec4(0.2f, 0.1f, 0.025f, 100000.0f);
    //gTheLights.theLights[2].direction = glm::vec4(0.0f, -1.0f, 1.0f, 1.0f);
    //gTheLights.theLights[0].specular = glm::vec4(1.0f, 1.0f, 1.0f, 50.0f);
    gTheLights.theLights[2].param1.x = 0;
    gTheLights.TurnOnLight(2);  // Or this!
    gTheLights.SetUpUniformLocations(program, 2);

    gTheLights.theLights[3].name = "Outside light";
    gTheLights.theLights[3].position = glm::vec4(6.5f, 5.5f, -8.5f, 1.0f);
    gTheLights.theLights[3].diffuse = glm::vec4(1.0f, 0.6f, .05f, 1.0f);
    gTheLights.theLights[3].atten = glm::vec4(0.2f, 0.1f, 0.025f, 100000.0f);
    //gTheLights.theLights[3].direction = glm::vec4(0.0f, -1.0f, 1.0f, 1.0f);
    //gTheLights.theLights[0].specular = glm::vec4(1.0f, 1.0f, 1.0f, 50.0f);
    gTheLights.theLights[3].param1.x = 0;
    gTheLights.TurnOnLight(3);  // Or this!
    gTheLights.SetUpUniformLocations(program, 3);

    gTheLights.theLights[6].name = "Outside light";
    gTheLights.theLights[6].position = glm::vec4(10.5f, 5.5f, -0.25f, 1.0f);
    gTheLights.theLights[6].diffuse = glm::vec4(1.0f, 0.6f, .05f, 1.0f);
    gTheLights.theLights[6].atten = glm::vec4(0.2f, 0.1f, 0.025f, 100000.0f);
    //gTheLights.theLights[3].direction = glm::vec4(0.0f, -1.0f, 1.0f, 1.0f);
    //gTheLights.theLights[0].specular = glm::vec4(1.0f, 1.0f, 1.0f, 50.0f);
    gTheLights.theLights[6].param1.x = 0;
    gTheLights.TurnOnLight(6);  // Or this!
    gTheLights.SetUpUniformLocations(program, 6);

    gTheLights.theLights[7].name = "Outside light";
    gTheLights.theLights[7].position = glm::vec4(-10.5f, 6.5f, 4.0f, 1.0f);
    gTheLights.theLights[7].diffuse = glm::vec4(1.0f, 0.6f, .05f, 1.0f);
    gTheLights.theLights[7].atten = glm::vec4(0.2f, 0.1f, 0.025f, 100000.0f);
    //gTheLights.theLights[3].direction = glm::vec4(0.0f, -1.0f, 1.0f, 1.0f);
    //gTheLights.theLights[0].specular = glm::vec4(1.0f, 1.0f, 1.0f, 50.0f);
    gTheLights.theLights[7].param1.x = 0;
    gTheLights.TurnOnLight(7);  // Or this!
    gTheLights.SetUpUniformLocations(program, 7);

    gTheLights.theLights[9].name = "Outside light";
    gTheLights.theLights[9].position = glm::vec4(4.25f, 6.0f, 26.5f, 1.0f);
    gTheLights.theLights[9].diffuse = glm::vec4(1.0f, 0.6f, .05f, 1.0f);
    gTheLights.theLights[9].atten = glm::vec4(0.2f, 0.1f, 0.025f, 100000.0f);
    //gTheLights.theLights[0].direction = glm::vec4(0.0f, -1.0f, 1.0f, 1.0f);
    //gTheLights.theLights[0].specular = glm::vec4(1.0f, 1.0f, 1.0f, 50.0f);
    gTheLights.theLights[9].param1.x = 0;
    gTheLights.TurnOnLight(9);  // Or this!
    gTheLights.SetUpUniformLocations(program, 9);


    //INSIDE LIGHTS
    gTheLights.theLights[4].name = "Inside light";
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

    gTheLights.theLights[5].name = "Inside light";
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

    gTheLights.theLights[8].name = "Sun light";
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
    gTheLights.theLights[12].param1.x = 2;
    gTheLights.TurnOffLight(12);  // Or this!
    gTheLights.SetUpUniformLocations(program, 12);

    sceneLoader = cSceneLoader::GetSceneLoaderInstance();

    //Gets the path of the texture manager to the texture folder
    g_textureManager.SetBasePath("assets/textures");

    std::string errorString;
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

   //g_textureManager.Create2DTextureFromBMPFile("smoke.bmp", true);
   //g_textureManager.Create2DTextureFromBMPFile("spyglass.bmp", true);

    gVAOManager.SetShaderProgramID_Threaded(program);
    sModelDrawInfo loadingModel;
    gVAOManager.LoadPendingModelIntoVAO("loading.ply", loadingModel);

    sModelDrawInfo fullscreenQuadInfo;
    if (!gVAOManager.LoadModelIntoVAO("fullscreenquad.ply", fullscreenQuadInfo, program))
    {
        std::cout << "Issue loading fullscreenquad" << std::endl;
    }

    //Loads the scene and the textures used in the scene
    std::cout << "Loading scene " << sceneName << "...." << std::endl;
    if (sceneLoader->LoadScene(sceneName, &g_textureManager, &g_entityManager))
    {
        std::cout << "Loaded scene: " << sceneName << std::endl << std::endl;
        std::cout << "Loading scene into VAO manager...." << std::endl;
        if (sceneLoader->LoadIntoVAO(&gVAOManager, program))
        {
            std::cout << "Loaded scene into VAO manager" << std::endl << std::endl;

            cameraEye = sceneLoader->GetCameraStartingPosition();
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

    //gets the sky box model so we can change the skybox texture
    g_skyBox = g_entityManager.GetEntityByName("Sky");

    g_entityManager.GetEntityByName("poolwater")->GetComponent<cMeshRenderer>()->bUseSkyboxReflection = true;

    cShaderManager::cShaderProgram* normalShader = gShaderManager.pGetShaderProgramFromFriendlyName("Shader#1");

    normalShader->uniformLocations.insert(std::pair<std::string, GLint>("matModel", glGetUniformLocation(program, "matModel")));
    normalShader->uniformLocations.insert(std::pair<std::string, GLint>("matModelInverseTranspose", glGetUniformLocation(program, "matModelInverseTranspose")));
    // Copy the whole object colour information to the sahder               

            // This is used for wireframe or whole object colour. 
            // If bUseDebugColour is TRUE, then the fragment colour is "objectDebugColour".
    normalShader->uniformLocations.insert(std::pair<std::string, GLint>("bUseDebugColour", glGetUniformLocation(program, "bUseDebugColour")));
    normalShader->uniformLocations.insert(std::pair<std::string, GLint>("objectDebugColour", glGetUniformLocation(program, "objectDebugColour")));

    // If true, then the lighting contribution is NOT used. 
    // This is useful for wireframe object
    normalShader->uniformLocations.insert(std::pair<std::string, GLint>("bDontLightObject", glGetUniformLocation(program, "bDontLightObject")));

    // The "whole object" colour (diffuse and specular)
   normalShader->uniformLocations.insert(std::pair<std::string, GLint>("wholeObjectDiffuseColour", glGetUniformLocation(program, "wholeObjectDiffuseColour")));
   normalShader->uniformLocations.insert(std::pair<std::string, GLint>("bUseWholeObjectDiffuseColour", glGetUniformLocation(program, "bUseWholeObjectDiffuseColour")));
   normalShader->uniformLocations.insert(std::pair<std::string, GLint>("wholeObjectSpecularColour", glGetUniformLocation(program, "wholeObjectSpecularColour")));
   normalShader->uniformLocations.insert(std::pair<std::string, GLint>("bUseSpecular", glGetUniformLocation(program, "bUseSpecular")));
   normalShader->uniformLocations.insert(std::pair<std::string, GLint>("eyeLocation", glGetUniformLocation(program, "eyeLocation")));

   normalShader->uniformLocations.insert(std::pair<std::string, GLint>("texture_00", glGetUniformLocation(program, "texture_00")));
   normalShader->uniformLocations.insert(std::pair<std::string, GLint>("texture_01", glGetUniformLocation(program, "texture_01")));
   normalShader->uniformLocations.insert(std::pair<std::string, GLint>("texture_02", glGetUniformLocation(program, "texture_02")));
   normalShader->uniformLocations.insert(std::pair<std::string, GLint>("texture_03", glGetUniformLocation(program, "texture_03")));

   normalShader->uniformLocations.insert(std::pair<std::string, GLint>("texLightpassColorBuf", glGetUniformLocation(program, "texLightpassColorBuf")));

   normalShader->uniformLocations.insert(std::pair<std::string, GLint>("texture_MatColor", glGetUniformLocation(program, "texture_MatColor")));
   normalShader->uniformLocations.insert(std::pair<std::string, GLint>("texture_Normal", glGetUniformLocation(program, "texture_Normal")));
   normalShader->uniformLocations.insert(std::pair<std::string, GLint>("texture_WorldPos", glGetUniformLocation(program, "texture_WorldPos")));
   normalShader->uniformLocations.insert(std::pair<std::string, GLint>("texture_Specular", glGetUniformLocation(program, "texture_Specular")));

   normalShader->uniformLocations.insert(std::pair<std::string, GLint>("textureRatios", glGetUniformLocation(program, "textureRatios")));

   normalShader->uniformLocations.insert(std::pair<std::string, GLint>("bUseAlphaMask", glGetUniformLocation(program, "bUseAlphaMask")));
   normalShader->uniformLocations.insert(std::pair<std::string, GLint>("alphaMask", glGetUniformLocation(program, "alphaMask")));

   normalShader->uniformLocations.insert(std::pair<std::string, GLint>("bUseNormalMap", glGetUniformLocation(program, "bUseNormalMap")));
   normalShader->uniformLocations.insert(std::pair<std::string, GLint>("normalMap", glGetUniformLocation(program, "normalMap")));
   normalShader->uniformLocations.insert(std::pair<std::string, GLint>("normalOffset", glGetUniformLocation(program, "normalOffset")));

   normalShader->uniformLocations.insert(std::pair<std::string, GLint>("bUseHeightMap", glGetUniformLocation(program, "bUseHeightMap")));
   normalShader->uniformLocations.insert(std::pair<std::string, GLint>("heightMap", glGetUniformLocation(program, "heightMap")));

   normalShader->uniformLocations.insert(std::pair<std::string, GLint>("bUseSkybox", glGetUniformLocation(program, "bUseSkybox")));
   normalShader->uniformLocations.insert(std::pair<std::string, GLint>("skyBox", glGetUniformLocation(program, "skyBox")));

   normalShader->uniformLocations.insert(std::pair<std::string, GLint>("bDebugMode", glGetUniformLocation(program, "bDebugMode")));
   normalShader->uniformLocations.insert(std::pair<std::string, GLint>("bDebugShowLighting", glGetUniformLocation(program, "bDebugShowLighting")));
   normalShader->uniformLocations.insert(std::pair<std::string, GLint>("bDebugShowNormals", glGetUniformLocation(program, "bDebugShowNormals")));

   normalShader->uniformLocations.insert(std::pair<std::string, GLint>("bIsImposter", glGetUniformLocation(program, "bIsImposter")));

   normalShader->uniformLocations.insert(std::pair<std::string, GLint>("passNumber", glGetUniformLocation(program, "passNumber")));
   normalShader->uniformLocations.insert(std::pair<std::string, GLint>("screenWidthHeight", glGetUniformLocation(program, "screenWidthHeight")));

   normalShader->uniformLocations.insert(std::pair<std::string, GLint>("bShowBloom", glGetUniformLocation(program, "bShowBloom")));
   normalShader->uniformLocations.insert(std::pair<std::string, GLint>("bUseSkyboxReflections", glGetUniformLocation(program, "bUseSkyboxReflections")));
   normalShader->uniformLocations.insert(std::pair<std::string, GLint>("bUseSkyboxRefraction", glGetUniformLocation(program, "bUseSkyboxRefraction")));

   normalShader->uniformLocations.insert(std::pair<std::string, GLint>("postprocessingVariables", glGetUniformLocation(program, "postprocessingVariables")));
   normalShader->uniformLocations.insert(std::pair<std::string, GLint>("emmisionPower", glGetUniformLocation(program, "emmisionPower")));
   normalShader->uniformLocations.insert(std::pair<std::string, GLint>("bloomMapColorBuf", glGetUniformLocation(program, "bloomMapColorBuf")));

   cShaderManager::cShaderProgram* pingPongShader = gShaderManager.pGetShaderProgramFromFriendlyName("PingPong");

   //glUseProgram(pingPongShader->ID);
   pingPongShader->uniformLocations.insert(std::pair<std::string, GLint>("screenWidthHeight", glGetUniformLocation(pingPongShader->ID, "screenWidthHeight")));
   pingPongShader->uniformLocations.insert(std::pair<std::string, GLint>("bloomMap", glGetUniformLocation(pingPongShader->ID, "bloomMap")));
   pingPongShader->uniformLocations.insert(std::pair<std::string, GLint>("horizontal", glGetUniformLocation(pingPongShader->ID, "horizontal")));

   pingPongShader->uniformLocations.insert(std::pair<std::string, GLint>("matModel", glGetUniformLocation(pingPongShader->ID, "matModel")));
   pingPongShader->uniformLocations.insert(std::pair<std::string, GLint>("matModelInverseTranspose", glGetUniformLocation(pingPongShader->ID, "matModelInverseTranspose")));

   pingPongShader->uniformLocations.insert(std::pair<std::string, GLint>("matView", glGetUniformLocation(pingPongShader->ID, "matView")));
   pingPongShader->uniformLocations.insert(std::pair<std::string, GLint>("matProjection", glGetUniformLocation(pingPongShader->ID, "matProjection")));
   pingPongShader->uniformLocations.insert(std::pair<std::string, GLint>("bUseHeightMap", glGetUniformLocation(pingPongShader->ID, "bUseHeightMap")));

   pingPongShader->uniformLocations.insert(std::pair<std::string, GLint>("bloomSize", glGetUniformLocation(pingPongShader->ID, "bloomSize")));

   glUniform1f(pingPongShader->uniformLocations["bUseHeightMap"], (float)GL_FALSE);

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

    const float MAX_DELTA_TIME = 0.1f;	// 100 ms
    float previousTime = (float)glfwGetTime();

    float ratio;
    int width, height;

    glfwGetFramebufferSize(window, &width, &height);
    ratio = width / (float)height;

    int screenPixelDensity = 2000;

    g_fbo = new cFBO();
    if (!g_fbo->init((int)(screenPixelDensity * ratio), screenPixelDensity, errorString))
    {
        std::cout << "Error in FBO" << std::endl;
    }

    float decreaseSize = 4.0f;

    pingPongFBO = new cPingPongFBOs((screenPixelDensity * ratio) / decreaseSize, screenPixelDensity / decreaseSize);

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    const GLint RENDER_PASS_0_G_BUFFER = 0;
    const GLint RENDER_PASS_1_LIGHTING = 1;
    const GLint RENDER_PASS_2_EFFECTS = 2;

    const GLint RENDER_PASS_BLUR_HORIZONTAL = 4;
    const GLint RENDER_PASS_BLUR_VERTICAL = 5;

    //GUI StUFF
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& getIo = ImGui::GetIO();
    //(void)io;
    io = &getIo;
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 420");

    ImGui::GetStyle().WindowRounding = 5.0f;

    io->Fonts->AddFontFromFileTTF("assets/fonts/Roboto-Medium.ttf", 15);

    while (!glfwWindowShouldClose(window))
    {
        glUniform1ui(normalShader->uniformLocations["passNumber"], RENDER_PASS_0_G_BUFFER);

        float useExposure = postProcessing.useExposureToneMapping ? 0.0f : 1.0f;
        glUniform4f(normalShader->uniformLocations["postprocessingVariables"], postProcessing.gamma, postProcessing.exposure, useExposure, postProcessing.bloomThreshhold);

        float currentTime = (float)glfwGetTime();
        float deltaTime = currentTime - previousTime;
        deltaTime = (deltaTime > MAX_DELTA_TIME ? MAX_DELTA_TIME : deltaTime);
        previousTime = currentTime;

        //glm::mat4 matModel;    // used to be "m"; Sometimes it's called "world"
        glm::mat4 p;
        glm::mat4 v;

        pingPongFBO->ClearBuffers();

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        g_fbo->clearBuffers(true, true);

        glBindFramebuffer(GL_FRAMEBUFFER, g_fbo->ID);
        g_fbo->clearBuffers(true, true);

        glViewport(0, 0, g_fbo->width, g_fbo->height);
        ratio = g_fbo->width / (float)g_fbo->height;

        glUniform2f(normalShader->uniformLocations["screenWidthHeight"], (GLfloat)g_fbo->width, (GLfloat)g_fbo->height);

        // Turn on the depth buffer
        glEnable(GL_DEPTH);         // Turns on the depth buffer
        glEnable(GL_DEPTH_TEST);    // Check if the pixel is already closer

   
       

        

        // *******************************************************
        // Screen is cleared and we are ready to draw the scene...
        // *******************************************************

        // Copy the light information into the shader to draw the scene
        gTheLights.CopyLightInfoToShader();

        // Place the "debug sphere" at the same location as the selected light (again)
        // HACK: Debug sphere is 5th item added
        float usedFov = fov;

        p = glm::perspective(glm::radians(usedFov),
            ratio,
            0.1f,
            1000.0f);     

        v = glm::mat4(1.0f);

        //glm::vec3 cameraEye = glm::vec3(0.0, 0.0, -4.0f);
        glm::vec3 cameraTarget = glm::vec3(0.0f, 0.0f, 0.0f);
        glm::vec3 upVector = glm::vec3(0.0f, 1.0f, 0.0f);

        v = glm::lookAt(cameraEye,     // "eye"
            cameraEye + cameraDir,  // "at"
            cameraUp);

        glUniform4f(normalShader->uniformLocations["eyeLocation"], cameraEye.x, cameraEye.y, cameraEye.z, 1.0f);

        glUniformMatrix4fv(matView_Location, 1, GL_FALSE, glm::value_ptr(v));
        glUniformMatrix4fv(matProjection_Location, 1, GL_FALSE, glm::value_ptr(p));

        //Sort based on transparency
        std::vector<cEntity*> transparentMeshes;
        std::vector<cEntity*> opaqueMeshes;

        std::vector<cEntity*> entityVector = g_entityManager.GetEntities();
        for (unsigned int index = 0; index != entityVector.size(); index++)
        {
            cEntity* curEntity = entityVector.at(index);

            cMeshRenderer* curMesh = curEntity->GetComponent<cMeshRenderer>();

            if (curMesh->wholeObjectDiffuseRGBA.a < 1.0f)
            {
                transparentMeshes.push_back(entityVector.at(index));
            }
            else
            {
                opaqueMeshes.push_back(curEntity);
            }
        }
        //Sort transparent objects.
        std::sort(transparentMeshes.begin(), transparentMeshes.end(), DistanceToCameraPredicate);
        
        
        Draw(opaqueMeshes, transparentMeshes, normalShader->uniformLocations, deltaTime);
       

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
       
       
        v = glm::lookAt(fullscreenPos,     // "eye"
            fullscreenPos + glm::vec3(0.f, 0.f, 1.0f) ,  // "at"
            cameraUp);
        glUniformMatrix4fv(matView_Location, 1, GL_FALSE, glm::value_ptr(v));
        
        p = glm::ortho(0.0f, 1.0f / (float)width, 0.0f, 1.0f / (float)height, 0.01f, 1000.0f);
        glUniformMatrix4fv(matProjection_Location, 1, GL_FALSE, glm::value_ptr(p));
        
        glUniform1ui(normalShader->uniformLocations["passNumber"], RENDER_PASS_1_LIGHTING);
        
        //Uploading textures to gpu
        GLint textureMatId = g_fbo->vertexMatColour_1_ID;
        if (textureMatId != 0)
        {
            GLint unit = 7;
            glActiveTexture(unit + GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, textureMatId);
            glUniform1i(normalShader->uniformLocations["texture_MatColor"], unit);
        }
        
        GLint textureNormalId = g_fbo->vertexNormal_2_ID;
        if (textureNormalId != 0)
        {
            GLint unit = 8;
            glActiveTexture(unit + GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, textureNormalId);
            glUniform1i(normalShader->uniformLocations["texture_Normal"], unit);
        }
        
        GLint textureWorldId = g_fbo->vertexWorldPos_3_ID;
        if (textureWorldId != 0)
        {
            GLint unit = 9;
            glActiveTexture(unit + GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, textureWorldId);
            glUniform1i(normalShader->uniformLocations["texture_WorldPos"], unit);
        }
        
        GLint textureSpecularId = g_fbo->vertexSpecular_4_ID;
        if (textureSpecularId != 0)
        {
            GLint unit = 10;
            glActiveTexture(unit + GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, textureSpecularId);
            glUniform1i(normalShader->uniformLocations["texture_Specular"], unit);
        }
        g_fbo->clearColourBuffer(0);  
        //g_fbo->clearColourBuffer(5);
        //fullscreenEntity->GetComponent<cTransform>()->position.z -= .1f;
        DrawObject(fullscreenEntity, glm::mat4(1.0f), program, &gVAOManager, g_textureManager, normalShader->uniformLocations, fullscreenPos);
        //DONE 1ST PASS (Lighting)
        
        //TODO: Add bloom pass
        
        bool horizontal = true;
        bool first_iteration = true;

        glUseProgram(pingPongShader->ID);
        
        glUniform2f(pingPongShader->uniformLocations["screenWidthHeight"], (GLfloat)pingPongFBO->width, (GLfloat)pingPongFBO->height);
        glUniformMatrix4fv(pingPongShader->uniformLocations["matView"], 1, GL_FALSE, glm::value_ptr(v));
        glUniformMatrix4fv(pingPongShader->uniformLocations["matProjection"], 1, GL_FALSE, glm::value_ptr(p));

        glUniform1f(pingPongShader->uniformLocations["bloomSize"], postProcessing.bloomSize);
        
        for (unsigned int i = 0; i < postProcessing.bloomIterationAmount; i++)
        {
            glBindFramebuffer(GL_FRAMEBUFFER, pingPongFBO->pingpongFBO[horizontal]);
            glUniform1f(pingPongShader->uniformLocations["horizontal"], horizontal ? (float)GL_TRUE : (float)GL_FALSE);
        
            if (first_iteration)
            {
                //glBindTexture(GL_TEXTURE_2D, g_fbo->brightColour_5_ID);
        
                GLint unit = 20;
                glActiveTexture(unit + GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, g_fbo->brightColour_5_ID);
                glUniform1i(pingPongShader->uniformLocations["bloomMap"], unit);
        
                first_iteration = false;
            }
            else
            {
                //glBindTexture(GL_TEXTURE_2D, pingPongFBO->pingpongBuffer[!horizontal]);
        
                GLint unit = 21;
                glActiveTexture(unit + GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, pingPongFBO->pingpongBuffer[!horizontal]);
                glUniform1i(pingPongShader->uniformLocations["bloomMap"], unit);
            }
        
            fullscreenEntity->GetComponent<cTransform>()->position.z -= .01f;
            DrawObject(fullscreenEntity, glm::mat4(1.0f), pingPongShader->ID, &gVAOManager, g_textureManager, pingPongShader->uniformLocations, fullscreenPos);
            horizontal = !horizontal;
        }
        glUseProgram(normalShader->ID);

        //BEGINNING OF SECOND PASS (Effects Pass)
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        //glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        //glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        glfwGetFramebufferSize(window, &width, &height);
        ratio = width / (float)height;
        
        p = glm::perspective(glm::radians(usedFov),
            ratio,
            0.1f,
            1000.0f);
        
        glViewport(0, 0, width, height);
        
        glUniform2f(normalShader->uniformLocations["screenWidthHeight"], width, height);
        glUniform1ui(normalShader->uniformLocations["passNumber"], RENDER_PASS_2_EFFECTS);
        
        GLint textureId = 0;
        
        switch (showTextureIndex)
        {
        case 0:
            textureId = g_fbo->colourTexture_0_ID;
            break;
        case 1:
            textureId = g_fbo->vertexMatColour_1_ID;
            break;
        case 2:
            textureId = g_fbo->vertexNormal_2_ID;
            break;
        case 3:
            textureId = g_fbo->vertexSpecular_4_ID;
            break;
        case 4:
            textureId = g_fbo->vertexWorldPos_3_ID;
            break;
        case 5:
            textureId = pingPongFBO->pingpongBuffer[!horizontal];
            break;
        }
        
        //Upload colour buffer
        if (textureId != 0)
        {
            GLint unit = 11;
            glActiveTexture(unit + GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, textureId);
            glUniform1i(normalShader->uniformLocations["texLightpassColorBuf"], unit);
        }

        //Only show bloom on the lit colour buffer
        glUniform1f(normalShader->uniformLocations["bShowBloom"], (textureId == g_fbo->colourTexture_0_ID) ? (float)GL_TRUE : (float)GL_FALSE);
        //Upload bloom map
        if (pingPongFBO->pingpongBuffer[!horizontal] != 0)
        {
            GLint unit = 12;
            glActiveTexture(unit + GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, pingPongFBO->pingpongBuffer[!horizontal]);
            glUniform1i(normalShader->uniformLocations["bloomMapColorBuf"], unit);
        }

        

        fullscreenEntity->GetComponent<cTransform>()->position.z -= .1f;
        DrawObject(fullscreenEntity, glm::mat4(1.0f), program, &gVAOManager, g_textureManager, normalShader->uniformLocations, fullscreenPos);
        //END OF FINAL PASS

        if(showDebugGui)
            DrawGUI(deltaTime);

        //glEnable(GL_CULL_FACE);
        //glCullFace(GL_BACK);

        g_entityManager.DeleteEntity(fullscreenEntity);
        // "Present" what we've drawn.
        glfwSwapBuffers(window);
        glfwPollEvents();

        ProcessAsyncMouse(window, (float)deltaTime);
        ProcessAsyncKeyboard(window, (float)deltaTime);

        //g_FlyCamera->Update(deltaTime);   

        //Flicker lights
        //gTheLights.theLights[0].atten.x = (rand() % 200 + 100) / 1000.0f;
        //gTheLights.theLights[1].atten.x = (rand() % 200 + 100) / 1000.0f;
        //gTheLights.theLights[2].atten.x = (rand() % 200 + 100) / 1000.0f;
        //gTheLights.theLights[3].atten.x = (rand() % 200 + 100) / 1000.0f;
        //gTheLights.theLights[6].atten.x = (rand() % 200 + 100) / 1000.0f;
        //gTheLights.theLights[7].atten.x = (rand() % 200 + 100) / 1000.0f;

    }

    g_entityManager.DeleteEntity(g_DebugSphere);

    if (!g_fbo->shutdown())
    {
        std::cout << "Error shutting down fbo" << std::endl;
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();

    ImGui::DestroyContext();

    delete g_fbo;
    delete pingPongFBO;

    glfwDestroyWindow(window);

    glfwTerminate();
    exit(EXIT_SUCCESS);
}

void Draw(std::vector<cEntity*> opaqueMeshes, std::vector<cEntity*> transparentMeshes, std::map<std::string, int> uniformLocations, float deltaTime)
{

    //Draw non transparent objects
    for (unsigned int index = 0; index != opaqueMeshes.size(); index++)
    {
        cEntity* curEntity = opaqueMeshes[index];
        glm::mat4 matModel = glm::mat4(1.0f);  // "Identity" ("do nothing", like x1)
        //mat4x4_identity(m);

        if (curEntity->GetComponent<cMeshRenderer>()->friendlyName == "Sky")
        {
            curEntity->GetComponent<cTransform>()->position = cameraEye;
        }

        DrawObject(curEntity, matModel, program, &gVAOManager, g_textureManager, uniformLocations, cameraEye);
    }//for (unsigned int index

    //glEnable(GL_BLEND);
    //glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    //Draw transparent objects
    for (unsigned int index = 0; index != transparentMeshes.size(); index++)
    {
        cEntity* curEntity = transparentMeshes[index];
        cMeshRenderer* curMesh = curEntity->GetComponent<cMeshRenderer>();
        glm::mat4 matModel = glm::mat4(1.0f);  // "Identity" ("do nothing", like x1)
        //mat4x4_identity(m);

        //Scroll pool normal effect
        if (curMesh->friendlyName == "poolwater")
        {
            curMesh->normalOffset.x += (float)deltaTime / 70.0f;
            curMesh->normalOffset.y += (float)deltaTime / 100.0f;
        }

        DrawObject(curEntity, matModel, program, &gVAOManager, g_textureManager, uniformLocations, cameraEye);
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
        DrawObject(g_DebugSphere, matModelDS, program, &gVAOManager, g_textureManager, uniformLocations, cameraEye);
    }
    else
    {
        glUniform1f(uniformLocations["bDebugMode"], (float)GL_FALSE);
    }
}

void DrawGUI(float dt)
{
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    // PP
    {
        ImGui::Begin("Post-Processing");
        ImGui::BeginChild("Post-Processing");
        ImGui::Separator();

        if (ImGui::BeginTabBar("##Tabs", ImGuiTabBarFlags_None))
        {
            if (ImGui::BeginTabItem("Visual"))
            {
                ImGui::SliderFloat("Gamma", &postProcessing.gamma, 0.0f, 5.0f);

                ImGui::Checkbox("Use Exposure", &postProcessing.useExposureToneMapping);
                if (postProcessing.useExposureToneMapping)
                    ImGui::SliderFloat("Exposure", &postProcessing.exposure, 0.0f, 5.0f);

                ImGui::DragFloat("Bloom Threshhold", &postProcessing.bloomThreshhold, 0.1f, 0.0f, 10000.0f);
                ImGui::DragFloat("Bloom Size", &postProcessing.bloomSize, 0.1f, 0.0f, 10.0f);

                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem("G-Buffer"))
            {
                if (ImGui::Selectable("Lit"))
                    showTextureIndex = 0;
                if (ImGui::Selectable("Diffuse"))
                    showTextureIndex = 1;
                if (ImGui::Selectable("Normals"))
                    showTextureIndex = 2;
                if (ImGui::Selectable("Specular"))
                    showTextureIndex = 3;
                if (ImGui::Selectable("World Position"))
                    showTextureIndex = 4;
                if (ImGui::Selectable("Bright Colours"))
                    showTextureIndex = 5;

                ImGui::EndTabItem();
            }
            ImGui::EndTabBar();
        }

        ImGui::EndChild();
        ImGui::End();
    }

    //Entites
    {
        ImGui::Begin("Entities");

        ImGui::BeginChild("left entity pane", ImVec2(150, 0), true);
        std::vector<cEntity*> entities = g_entityManager.GetEntities();
        for (size_t i = 0; i < entities.size(); i++)
        {
            if (ImGui::Selectable(std::string(entities[i]->name + ": " + std::to_string(i)).c_str(), selectedEntityDebug == i))
                selectedEntityDebug = i;
        }
        ImGui::EndChild();

        ImGui::SameLine();

        cEntity* curEntity = entities[selectedEntityDebug];
        ImGui::BeginChild("entity view", ImVec2(0, -ImGui::GetFrameHeightWithSpacing())); // Leave room for 1 line below us
        ImGui::Text(std::string(curEntity->name + ": %d").c_str(), selectedEntityDebug);
        ImGui::Separator();
        if (ImGui::BeginTabBar("##Tabs", ImGuiTabBarFlags_None))
        {
            if (ImGui::BeginTabItem("Transform"))
            {
                cTransform* curTransform = curEntity->GetComponent<cTransform>();
                ImGui::Text("Position");
                ImGui::DragFloat("pos x", &curTransform->position.x, 0.1f);
                ImGui::DragFloat("pos y", &curTransform->position.y, 0.1f);
                ImGui::DragFloat("pos z", &curTransform->position.z, 0.1f);

                glm::vec3 angleRotation = curTransform->GetEulerRotation();
                ImGui::Text("Rotation");
                ImGui::DragFloat("rot x", &angleRotation.x, 0.01f);
                ImGui::DragFloat("rot y", &angleRotation.y, 0.01f);
                ImGui::DragFloat("rot z", &angleRotation.z, 0.01f);
                curTransform->SetRotation(angleRotation);

                ImGui::Text("Scale");
                ImGui::DragFloat("scale x", &curTransform->scale.x, 0.1f);
                ImGui::DragFloat("scale y", &curTransform->scale.y, 0.1f);
                ImGui::DragFloat("scale z", &curTransform->scale.z, 0.1f);

                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem("Visuals"))
            {
                cMeshRenderer* renderer = curEntity->GetComponent<cMeshRenderer>();
                ImGui::DragFloat("Emmision", &renderer->emmision, 0.1f, 1.0f, 10000.0f);

                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem("Details"))
            {
                cMeshRenderer* renderer = curEntity->GetComponent<cMeshRenderer>();

                ImGui::Text(std::string("Mesh Name: " + renderer->meshName).c_str());
                ImGui::Text(std::string("Friendly Name: " + renderer->friendlyName).c_str());

                ImGui::EndTabItem();
            }
            ImGui::EndTabBar();
        }
        ImGui::EndChild();

        ImGui::End();
    }

    //Lights
    {
        ImGui::Begin("Lights");

        ImGui::BeginChild("left light pane", ImVec2(150, 0), true);
        for (size_t i = 0; i < gTheLights.NUMBER_OF_LIGHTS; i++)
        {
            if (ImGui::Selectable(std::string(gTheLights.theLights[i].name + ": " + std::to_string(i)).c_str(), selectedLightDebug == i))
                selectedLightDebug = i;
        }
        ImGui::EndChild();

        ImGui::SameLine();

        sLight* curLight = &gTheLights.theLights[selectedLightDebug];
        ImGui::BeginChild("light view", ImVec2(0, -ImGui::GetFrameHeightWithSpacing())); // Leave room for 1 line below us
        ImGui::Text(std::string(curLight->name + ": %d").c_str(), selectedLightDebug);
        ImGui::Separator();
        if (ImGui::BeginTabBar("##Tabs", ImGuiTabBarFlags_None))
        {
            if (ImGui::BeginTabItem("Transform"))
            {
                ImGui::Text("Position");
                ImGui::DragFloat("pos x", &curLight->position.x);
                ImGui::DragFloat("pos y", &curLight->position.y);
                ImGui::DragFloat("pos z", &curLight->position.z);

                ImGui::EndTabItem();
            }

            if (ImGui::BeginTabItem("Lighting"))
            {
                ImGui::Checkbox("On", &curLight->on);

                float colors[3] = { curLight->diffuse.x, curLight->diffuse.y, curLight->diffuse.z };
                ImGui::ColorEdit3("Diffuse", colors);
                curLight->diffuse.x = colors[0];
                curLight->diffuse.y = colors[1];
                curLight->diffuse.z = colors[2];

                ImGui::Text("Attenuation");
                ImGui::DragFloat("atten x", &curLight->atten.x, 0.01f, 0.0f, 3.0f);
                ImGui::DragFloat("atten y", &curLight->atten.y, 0.01f, 0.0f, 3.0f);
                ImGui::DragFloat("atten z", &curLight->atten.z, .01f, 0.0f, 3.0f);
                ImGui::SliderFloat("distance cutoff", &curLight->atten.w, 0.0f, 10000.0f);

                ImGui::Text("Light Power");
                ImGui::DragFloat("power", &curLight->power, 1.0f, 0.0f, 10000.0f);

                ImGui::EndTabItem();
            }

            ImGui::EndTabBar();
        }
        ImGui::EndChild();

        ImGui::End();
    }

    //Misc
    {
        ImGui::Begin("Misc");
        ImGui::Text(std::string("Frame time: %f").c_str(), dt);
        ImGui::Text("Mobius Engine by Ethan Robertson");
        ImGui::End();
    }

    //ImGui::ShowDemoWindow();

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    ImGui::EndFrame();
}