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
#include "cShadowDepthFBO.h"

#include "RenderType.h"
#include "cInstancedRenderer.h"
#include "cInstancedBrush.h"

struct PostProcessingInfo
{
    float gamma = 1.85f;
    float exposure = 0.631f;
    float shadowBias = 0.00124f;

    bool useExposureToneMapping = true;

    float bloomThreshhold = 5.0f;
    float bloomSize = 1.0f;
    unsigned int bloomIterationAmount = 15;

    float ambientPower = 0.1f;
};

struct WindInfo
{
    float strength = 1.1f;
    float size = 0.05f;
    glm::vec3 windDir = glm::vec3(1.0f, 0.1f, 1.0f);
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
float speedMultiple = 3.0f;

cVAOManager     gVAOManager;
cShaderManager  gShaderManager;

cEntityManager g_entityManager;

cLightManager gTheLights;
cLightHelper gTheLightHelper;

cBasicTextureManager g_textureManager;

cEntity* g_DebugSphere = NULL;

unsigned int g_selectedObject = 0;
unsigned int g_selectedLight = 0;

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

size_t selectedModelDebug = 0;

float fov = 70.0f;

PostProcessingInfo postProcessing;
WindInfo windInfo;

cInstancedBrush brush;

cPingPongFBOs* pingPongFBO;
cShadowDepthFBO* shadowFBO;

int instancedRenderOffsetAmount = 10;

bool g_MouseIsInsideWindow = false;

//Method in DrawObjectFunction
void extern DrawObject(cEntity* curEntity, glm::mat4 matModel, cShaderManager::cShaderProgram* shader, cVAOManager* VAOManager,
    cBasicTextureManager textureManager, glm::vec3 eyeLocation);

void Draw(std::vector<cEntity*>* opaqueMeshes, std::vector<cEntity*>* transparentMeshes, cShaderManager::cShaderProgram* program, float deltaTime);
void DrawGUI(float dt);
void SetUpLights();

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


        if ((glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS)
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
        else if ((glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
            && g_MouseIsInsideWindow)
        {
            if (brush.HasRenderer())
            {
                glBindFramebuffer(GL_FRAMEBUFFER, g_fbo->ID);

                int windowWidth, windowHeight;
                glfwGetWindowSize(window, &windowWidth, &windowHeight);

                float fixedYValue = (float)abs(y - (double)windowHeight);

                float widthRatio = x / windowWidth;
                float heightRatio = fixedYValue / windowHeight;

                glm::vec2 curPixelPos = glm::vec2(g_fbo->width * widthRatio, g_fbo->height * heightRatio);

                float pixel[4];

                //world pos buffer
                glReadBuffer(GL_COLOR_ATTACHMENT3);
                glReadPixels(curPixelPos.x, curPixelPos.y, 1, 1, GL_RGBA, GL_FLOAT, pixel);

                //std::cout << "x: " << g_fbo->width * widthRatio << " | y: " << g_fbo->height * heightRatio << std::endl;
                //std::cout << "r: " << pixel[0] << " | g: " << pixel[1] << " | b: " << pixel[2] << " | a: " << pixel[3] << std::endl;
                glBindFramebuffer(GL_FRAMEBUFFER, 0);

                glm::vec3 pixelWorldPos = glm::vec3(pixel[0], pixel[1], pixel[2]);
                brush.AddOffset(pixelWorldPos);
                
                //RAY CAST IDEA

                //normalize(raycastDirection = pixelWorldPos - cameraEye);
                //vec3 Start = cameraEye;
                //vec3 End = raycastDirection * rayLength;
            }
        }
    }
    return;
}

void ProcessAsyncKeyboard(GLFWwindow* window, float deltaTime)
{
    if (!io->WantCaptureKeyboard)
    {

        if (glfwGetKey(window, GLFW_KEY_LEFT_ALT) == GLFW_RELEASE
            && glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_RELEASE)
        {
            float speed = flyCameraSpeed;
            if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_REPEAT || glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
            {
                speed *= 3.0f;
            }

            if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS && glfwGetKey(window, GLFW_KEY_S) != GLFW_PRESS)
            {
                cameraEye += cameraDir * speed * deltaTime;
            }
            else if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS && glfwGetKey(window, GLFW_KEY_W) != GLFW_PRESS)
            {
                cameraEye -= cameraDir * speed * deltaTime;
            }

            if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS && glfwGetKey(window, GLFW_KEY_D) != GLFW_PRESS)
            {
                cameraEye += cameraRight * speed * deltaTime;
            }
            else if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS && glfwGetKey(window, GLFW_KEY_A) != GLFW_PRESS)
            {
                cameraEye -= cameraRight * speed * deltaTime;
            }

            if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS && glfwGetKey(window, GLFW_KEY_E) != GLFW_PRESS)
            {
                cameraEye -= glm::vec3(0.0f, 1.0f, 0.0f) * speed * deltaTime;
            }
            else if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS && glfwGetKey(window, GLFW_KEY_Q) != GLFW_PRESS)
            {
                cameraEye += glm::vec3(0.0f, 1.0f, 0.0f) * speed * deltaTime;
            }
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

    cShaderManager::cShader shadowVertShader;
    shadowVertShader.fileName = "assets/shaders/shadowVert.glsl";

    cShaderManager::cShader fragShader;
    fragShader.fileName = "assets/shaders/fragShader_01.glsl";

    cShaderManager::cShader shadowFragShader;
    shadowFragShader.fileName = "assets/shaders/shadowFrag.glsl";

    cShaderManager::cShader pingPongFragShader;
    pingPongFragShader.fileName = "assets/shaders/pingpongShader.glsl";

    if (gShaderManager.createProgramFromFile("Shader#1", vertShader, fragShader, RenderType::Normal))
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

    if (gShaderManager.createProgramFromFile("PingPong", vertShader, pingPongFragShader, RenderType::PingPong))
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

    if (gShaderManager.createProgramFromFile("Shadow", vertShader, shadowFragShader, RenderType::Shadow))
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
    
    SetUpLights();

    sceneLoader = cSceneLoader::GetSceneLoaderInstance();

    //Gets the path of the texture manager to the texture folder
    g_textureManager.SetBasePath("assets/textures");

    std::string errorString;
    //if (!g_textureManager.CreateCubeTextureFromBMPFiles("NightSky",
    //    "SpaceBox_right1_posX.bmp",
    //    "SpaceBox_left2_negX.bmp",
    //    "SpaceBox_top3_posY.bmp",
    //    "SpaceBox_bottom4_negY.bmp",
    //    "SpaceBox_front5_posZ.bmp",
    //    "SpaceBox_back6_negZ.bmp",
    //    //Is seamless
    //    true, errorString))
    //{
    //    std::cout << errorString << std::endl;
    //}

    if (!g_textureManager.CreateCubeTextureFromBMPFiles("NightSky",
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

    if (!g_textureManager.Create2DTextureFromBMPFile("noise.bmp", true))
    {
        std::cout << "Issue loading wind noise" << std::endl;
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
    normalShader->uniformLocations.insert(std::pair<std::string, GLint>("texture_LightSpacePos", glGetUniformLocation(program, "texture_LightSpacePos")));

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
    normalShader->uniformLocations.insert(std::pair<std::string, GLint>("ambientPower", glGetUniformLocation(program, "ambientPower")));

    normalShader->uniformLocations.insert(std::pair<std::string, GLint>("tilingAndOffset", glGetUniformLocation(program, "tilingAndOffset")));
    normalShader->uniformLocations.insert(std::pair<std::string, GLint>("bUseInstancedRendering", glGetUniformLocation(program, "bUseInstancedRendering")));

    normalShader->uniformLocations.insert(std::pair<std::string, GLint>("shadowMapColorBuf", glGetUniformLocation(program, "shadowMapColorBuf")));
    normalShader->uniformLocations.insert(std::pair<std::string, GLint>("shadowBias", glGetUniformLocation(program, "shadowBias")));
    normalShader->uniformLocations.insert(std::pair<std::string, GLint>("lightSpaceMatrix", glGetUniformLocation(program, "lightSpaceMatrix")));

    normalShader->uniformLocations.insert(std::pair<std::string, GLint>("bUseWind", glGetUniformLocation(program, "bUseWind")));
    normalShader->uniformLocations.insert(std::pair<std::string, GLint>("windMap", glGetUniformLocation(program, "windMap")));
    normalShader->uniformLocations.insert(std::pair<std::string, GLint>("windDirection", glGetUniformLocation(program, "windDirection")));
    normalShader->uniformLocations.insert(std::pair<std::string, GLint>("windTime", glGetUniformLocation(program, "windTime")));
    normalShader->uniformLocations.insert(std::pair<std::string, GLint>("windStrength", glGetUniformLocation(program, "windStrength")));
    normalShader->uniformLocations.insert(std::pair<std::string, GLint>("windSize", glGetUniformLocation(program, "windSize")));

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
    pingPongShader->uniformLocations.insert(std::pair<std::string, GLint>("bUseInstancedRendering", glGetUniformLocation(pingPongShader->ID, "bUseInstancedRendering")));

    pingPongShader->uniformLocations.insert(std::pair<std::string, GLint>("bUseWind", glGetUniformLocation(pingPongShader->ID, "bUseWind")));
    pingPongShader->uniformLocations.insert(std::pair<std::string, GLint>("windMap", glGetUniformLocation(pingPongShader->ID, "windMap")));
    pingPongShader->uniformLocations.insert(std::pair<std::string, GLint>("windDirection", glGetUniformLocation(pingPongShader->ID, "windDirection")));
    pingPongShader->uniformLocations.insert(std::pair<std::string, GLint>("windTime", glGetUniformLocation(pingPongShader->ID, "windTime")));
    pingPongShader->uniformLocations.insert(std::pair<std::string, GLint>("windStrength", glGetUniformLocation(pingPongShader->ID, "windStrength")));
    pingPongShader->uniformLocations.insert(std::pair<std::string, GLint>("windSize", glGetUniformLocation(pingPongShader->ID, "windSize")));

    cShaderManager::cShaderProgram* shadowShader = gShaderManager.pGetShaderProgramFromFriendlyName("Shadow");

    shadowShader->uniformLocations.insert(std::pair<std::string, GLint>("matModel", glGetUniformLocation(shadowShader->ID, "matModel")));
    shadowShader->uniformLocations.insert(std::pair<std::string, GLint>("matModelInverseTranspose", glGetUniformLocation(shadowShader->ID, "matModelInverseTranspose")));

    shadowShader->uniformLocations.insert(std::pair<std::string, GLint>("matView", glGetUniformLocation(shadowShader->ID, "matView")));
    shadowShader->uniformLocations.insert(std::pair<std::string, GLint>("matProjection", glGetUniformLocation(shadowShader->ID, "matProjection")));
    shadowShader->uniformLocations.insert(std::pair<std::string, GLint>("bUseHeightMap", glGetUniformLocation(shadowShader->ID, "bUseHeightMap")));
    shadowShader->uniformLocations.insert(std::pair<std::string, GLint>("bUseInstancedRendering", glGetUniformLocation(shadowShader->ID, "bUseInstancedRendering")));

    shadowShader->uniformLocations.insert(std::pair<std::string, GLint>("bUseWind", glGetUniformLocation(shadowShader->ID, "bUseWind")));
    shadowShader->uniformLocations.insert(std::pair<std::string, GLint>("windMap", glGetUniformLocation(shadowShader->ID, "windMap")));
    shadowShader->uniformLocations.insert(std::pair<std::string, GLint>("windDirection", glGetUniformLocation(shadowShader->ID, "windDirection")));
    shadowShader->uniformLocations.insert(std::pair<std::string, GLint>("windTime", glGetUniformLocation(shadowShader->ID, "windTime")));
    shadowShader->uniformLocations.insert(std::pair<std::string, GLint>("windStrength", glGetUniformLocation(shadowShader->ID, "windStrength")));
    shadowShader->uniformLocations.insert(std::pair<std::string, GLint>("windSize", glGetUniformLocation(shadowShader->ID, "windSize")));
    
    //Setup instanced renderers;
    {
        std::vector<cEntity*> entities = g_entityManager.GetEntities();
        for (cEntity* entity : entities)
        {        
            cInstancedRenderer* instanceRenderer = entity->GetComponent<cInstancedRenderer>();

            if (instanceRenderer != nullptr)
            {
                sModelDrawInfo drawInfo;

                cMeshRenderer* mesh = entity->GetComponent<cMeshRenderer>();
                gVAOManager.FindDrawInfoByModelName(mesh->meshName, drawInfo);

                instanceRenderer->SetupVertexArrayAttrib(&drawInfo);
            }
        }
    }



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

    pingPongFBO = new cPingPongFBOs((screenPixelDensity * ratio) / decreaseSize, screenPixelDensity / decreaseSize, pingPongShader);
    shadowFBO = new cShadowDepthFBO();

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    const GLint RENDER_PASS_0_G_BUFFER = 0;
    const GLint RENDER_PASS_1_LIGHTING = 1;
    const GLint RENDER_PASS_2_EFFECTS = 2;
    const GLint RENDER_PASS_SHADOW = 3;

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
        GLenum err;

        float useExposure = postProcessing.useExposureToneMapping ? 0.0f : 1.0f;
        glUniform4f(normalShader->uniformLocations["postprocessingVariables"], postProcessing.gamma, postProcessing.exposure, useExposure, postProcessing.bloomThreshhold);
        glUniform1f(normalShader->uniformLocations["ambientPower"], postProcessing.ambientPower);
        glUniform1f(normalShader->uniformLocations["shadowBias"], postProcessing.shadowBias);

        float currentTime = (float)glfwGetTime();
        float deltaTime = currentTime - previousTime;
        deltaTime = (deltaTime > MAX_DELTA_TIME ? MAX_DELTA_TIME : deltaTime);
        previousTime = currentTime;

        //glm::mat4 matModel;    // used to be "m"; Sometimes it's called "world"
        glm::mat4 p;
        glm::mat4 v;

        pingPongFBO->ClearBuffers();
        shadowFBO->ClearShadowBuffer();

        //Clear normal buffers
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        g_fbo->clearBuffers(true, true);

        //Clear fbo buffers
        glBindFramebuffer(GL_FRAMEBUFFER, g_fbo->ID);
        g_fbo->clearBuffers(true, true);

        // Turn on the depth buffer
        glEnable(GL_DEPTH_TEST);    // Check if the pixel is already closer

        // *******************************************************
        // Screen is cleared and we are ready to draw the scene...
        // *******************************************************

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
        // Copy the light information into the shader to draw the scene
        gTheLights.CopyLightInfoToShader();

        //glEnable(GL_CULL_FACE);
        //glCullFace(GL_FRONT);

        //RENDER SHADOW
        glUseProgram(shadowShader->ID);
        glBindFramebuffer(GL_FRAMEBUFFER, shadowFBO->depthFBO_ID);
        glViewport(0, 0, shadowFBO->SHADOW_WIDTH, shadowFBO->SHADOW_HEIGHT);

        //glUniform1ui(normalShader->uniformLocations["passNumber"], RENDER_PASS_SHADOW);
        float near_plane = .1f, far_plane = 70.5f;
        float shadowOrthoBounds = 40.0f;
        p = glm::ortho(-shadowOrthoBounds, shadowOrthoBounds, -shadowOrthoBounds, shadowOrthoBounds, near_plane, far_plane);


        glm::vec4 sunPos = gTheLights.theLights[8].position;
        glm::vec3 sunPosVec3 = glm::vec3(sunPos.x, sunPos.y, sunPos.z);
        glm::vec4 sunDir = gTheLights.theLights[8].direction;
        glm::vec3 sunDirVec3 = glm::vec3(sunDir.x, sunDir.y, sunDir.z);

        glm::vec3 sunLookAtDir = sunPosVec3 + sunDirVec3;
                                    //look at center of scene
        v = glm::lookAt(sunPosVec3, sunLookAtDir, glm::vec3(0.0f, 1.0f, 0.0f));

        glm::mat4 lightSpaceMat = p * v;
        //glm::mat4 lightSpaceMat = v * p;

        glUniformMatrix4fv(shadowShader->uniformLocations["matView"], 1, GL_FALSE, glm::value_ptr(v));
        glUniformMatrix4fv(shadowShader->uniformLocations["matProjection"], 1, GL_FALSE, glm::value_ptr(p));
        glUniform1f(shadowShader->uniformLocations["bUseInstancedRendering"], (float)GL_FALSE);
        glUniform1f(shadowShader->uniformLocations["bUseHeightMap"], (float)GL_FALSE);

        Draw(&opaqueMeshes, &transparentMeshes, shadowShader, deltaTime);

        //Render normal scene
        //glCullFace(GL_BACK);

        glUseProgram(normalShader->ID);
        glBindFramebuffer(GL_FRAMEBUFFER, g_fbo->ID);
        glViewport(0, 0, g_fbo->width, g_fbo->height);

        glUniform2f(normalShader->uniformLocations["screenWidthHeight"], (GLfloat)g_fbo->width, (GLfloat)g_fbo->height);
        glUniform4f(normalShader->uniformLocations["eyeLocation"], cameraEye.x, cameraEye.y, cameraEye.z, 1.0f);

        glUniformMatrix4fv(normalShader->uniformLocations["lightSpaceMatrix"], 1, GL_FALSE, glm::value_ptr(lightSpaceMat));

        //RENDER SCENE
        glUniform1ui(normalShader->uniformLocations["passNumber"], RENDER_PASS_0_G_BUFFER);
        //Draw scene
        float usedFov = fov;

        ratio = g_fbo->width / (float)g_fbo->height;

        p = glm::perspective(glm::radians(usedFov),
            ratio,
            0.1f,
            1000.0f);     
        glUniformMatrix4fv(matProjection_Location, 1, GL_FALSE, glm::value_ptr(p));
        //glm::vec3 cameraEye = glm::vec3(0.0, 0.0, -4.0f);
        glm::vec3 cameraTarget = glm::vec3(0.0f, 0.0f, 0.0f);
        glm::vec3 upVector = glm::vec3(0.0f, 1.0f, 0.0f);

        v = glm::lookAt(cameraEye,     // "eye"
            cameraEye + cameraDir,  // "at"
            cameraUp);
        glUniformMatrix4fv(matView_Location, 1, GL_FALSE, glm::value_ptr(v)); 
        //Draw scene
        Draw(&opaqueMeshes, &transparentMeshes, normalShader, deltaTime);    
        //End of zeroth pass

        
        //Lighting Pass
        glUniform1ui(normalShader->uniformLocations["passNumber"], RENDER_PASS_1_LIGHTING);
        glm::vec3 fullscreenPos = glm::vec3(0.f, 0.f, -10.f);
        
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

        GLint textureShadowId = shadowFBO->depthMap;
        if (textureShadowId != 0)
        {
            GLint unit = 25;
            glActiveTexture(unit + GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, textureShadowId);
            glUniform1i(normalShader->uniformLocations["shadowMapColorBuf"], unit);
        }

        GLint textureLightSpace = g_fbo->vertexLightSpacePos_7_ID;
        if (textureLightSpace != 0)
        {
            GLint unit = 26;
            glActiveTexture(unit + GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, textureLightSpace);
            glUniform1i(normalShader->uniformLocations["texture_LightSpacePos"], unit);
        }

        g_fbo->clearColourBuffer(0);  

        //fullscreenEntity->GetComponent<cTransform>()->position.z -= .1f;
        DrawObject(fullscreenEntity, glm::mat4(1.0f), normalShader, &gVAOManager, g_textureManager, fullscreenPos);
        //DONE 1ST PASS (Lighting)
        
        bool horizontal = true;
        bool firstIteration = true;

        glUseProgram(pingPongShader->ID);
        
        glUniform2f(pingPongShader->uniformLocations["screenWidthHeight"], (GLfloat)pingPongFBO->width, (GLfloat)pingPongFBO->height);
        glUniformMatrix4fv(pingPongShader->uniformLocations["matView"], 1, GL_FALSE, glm::value_ptr(v));
        glUniformMatrix4fv(pingPongShader->uniformLocations["matProjection"], 1, GL_FALSE, glm::value_ptr(p));

        glUniform1f(pingPongShader->uniformLocations["bloomSize"], postProcessing.bloomSize);
        
        for (unsigned int i = 0; i < postProcessing.bloomIterationAmount; i++)
        {
            if (firstIteration)
            {
                pingPongFBO->BlurBuffer(true, horizontal, g_fbo->brightColour_5_ID);
                firstIteration = false;
            }
            else
            {
                pingPongFBO->BlurBuffer(false, horizontal);
            }
        
            fullscreenEntity->GetComponent<cTransform>()->position.z -= .01f;
            DrawObject(fullscreenEntity, glm::mat4(1.0f), pingPongShader, &gVAOManager, g_textureManager, fullscreenPos);
            horizontal = !horizontal;
        }
        glUseProgram(normalShader->ID);

        //BEGINNING OF SECOND PASS (Effects Pass)
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        glfwGetFramebufferSize(window, &width, &height);
        ratio = width / (float)height;
        
        glViewport(0, 0, width, height);
        
        glUniform2f(normalShader->uniformLocations["screenWidthHeight"], width, height);
        glUniform1ui(normalShader->uniformLocations["passNumber"], RENDER_PASS_2_EFFECTS);
        
        GLint textureId = 0;
        //Select which texture to show
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
        case 6:
            //This is the fbo object so that we can see the buffer later (it gets written to in the light pass)
            textureId = g_fbo->dirShadow_6_ID;
            break;
        case 7:
            textureId = g_fbo->vertexLightSpacePos_7_ID;
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
        DrawObject(fullscreenEntity, glm::mat4(1.0f), normalShader, &gVAOManager, g_textureManager, fullscreenPos);
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

        while ((err = glGetError()) != GL_NO_ERROR)
        {
            std::cout << "WARNING: OpenGL errors found!: " << err << std::endl;
        }
    }

    GLenum err;
    while ((err = glGetError()) != GL_NO_ERROR)
    {
        std::cout << "WARNING: OpenGL errors found!: " << err << std::endl;
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
    delete shadowFBO;

    glfwDestroyWindow(window);

    glfwTerminate();
    exit(EXIT_SUCCESS);
}

void Draw(std::vector<cEntity*>* opaqueMeshes, std::vector<cEntity*>* transparentMeshes, cShaderManager::cShaderProgram* program, float deltaTime)
{
    GLenum err;

    //Upload wind map
    GLint windNoise = g_textureManager.getTextureIDFromName("noise.bmp");
    if (windNoise != 0)
    {
        GLint unit = 27;
        glActiveTexture(unit + GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, windNoise);
        glUniform1i(program->uniformLocations["windMap"], unit);
    }

    glUniform3f(program->uniformLocations["windDirection"], windInfo.windDir.x, windInfo.windDir.y, windInfo.windDir.z);
    glUniform1f(program->uniformLocations["windTime"], (float)glfwGetTime());
    glUniform1f(program->uniformLocations["windStrength"], windInfo.strength);
    glUniform1f(program->uniformLocations["windSize"], windInfo.size);

    //Draw non transparent objects
    for (unsigned int index = 0; index != opaqueMeshes->size(); index++)
    {
        cEntity* curEntity = opaqueMeshes->at(index);
        glm::mat4 matModel = glm::mat4(1.0f);  // "Identity" ("do nothing", like x1)
        //mat4x4_identity(m);

        if (curEntity->GetComponent<cMeshRenderer>()->friendlyName == "Sky")
        {
            curEntity->GetComponent<cTransform>()->position = cameraEye;
        }

        DrawObject(curEntity, matModel, program, &gVAOManager, g_textureManager, cameraEye);
    }//for (unsigned int index

    //glEnable(GL_BLEND);
    //glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    //Draw transparent objects
    for (unsigned int index = 0; index != transparentMeshes->size(); index++)
    {
        cEntity* curEntity = transparentMeshes->at(index);
        cMeshRenderer* curMesh = curEntity->GetComponent<cMeshRenderer>();
        glm::mat4 matModel = glm::mat4(1.0f);  // "Identity" ("do nothing", like x1)
        //mat4x4_identity(m);

        //Scroll pool normal effect
        if (curMesh->friendlyName == "poolwater")
        {
            curMesh->normalOffset.x += (float)deltaTime / 70.0f;
            curMesh->normalOffset.y += (float)deltaTime / 100.0f;
        }

        DrawObject(curEntity, matModel, program, &gVAOManager, g_textureManager, cameraEye);
    }//for (unsigned int index
   
    //Render debug sphere
    if (isDebugMode)
    {
        glUniform1f(program->uniformLocations["bDebugMode"], (float)GL_TRUE);

        glUniform1f(program->uniformLocations["bDebugShowLighting"], (float)debugShowLighting);
        glUniform1f(program->uniformLocations["bDebugShowNormals"], (float)debugShowNormals);

        cMeshRenderer* debugSphereMesh = g_DebugSphere->GetComponent<cMeshRenderer>();
        cTransform* debugSphereTransform = g_DebugSphere->GetComponent<cTransform>();

        debugSphereTransform->position = gTheLights.theLights[g_selectedLight].position;
        debugSphereTransform->scale = glm::vec3(0.01f);
        debugSphereMesh->bDontLight = true;
        debugSphereMesh->bUseObjectDebugColour = true;
        debugSphereMesh->objectDebugColourRGBA = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);

        glm::mat4 matModelDS = glm::mat4(1.0f);
        DrawObject(g_DebugSphere, matModelDS, program, &gVAOManager, g_textureManager, cameraEye);
    }
    else
    {
        glUniform1f(program->uniformLocations["bDebugMode"], (float)GL_FALSE);
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

                ImGui::Text("Bloom");
                ImGui::Spacing();
                ImGui::DragFloat("Bloom Threshhold", &postProcessing.bloomThreshhold, 0.1f, 0.0f, 10000.0f);
                ImGui::DragFloat("Bloom Size", &postProcessing.bloomSize, 0.1f, 0.0f, 10.0f);

                ImGui::Text("Shadows");
                ImGui::Spacing();
                ImGui::DragFloat("Ambient Power", &postProcessing.ambientPower, 0.01f, 0.0f, 1.0f);
                ImGui::DragFloat("ShadowBias", &postProcessing.shadowBias, 0.0001f, 0.0f, 1.f, "%.5f");

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
                if (ImGui::Selectable("Light Space Position"))
                    showTextureIndex = 7;
                if (ImGui::Selectable("Bright Colours"))
                    showTextureIndex = 5;
                if (ImGui::Selectable("Shadow Map"))
                    showTextureIndex = 6;
                

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

        if (ImGui::BeginTabBar("EntityBar", ImGuiTabBarFlags_None))
        {
            if (ImGui::BeginTabItem("Entity List"))
            {
                ImGui::BeginChild("left entity pane", ImVec2(150, 0), true);
                std::vector<cEntity*> entities = g_entityManager.GetEntities();
                size_t entitiesSize = entities.size();
                if (selectedEntityDebug >= entitiesSize)
                {
                    selectedEntityDebug = entitiesSize - 1;
                }

                for (size_t i = 0; i < entitiesSize; i++)
                {
                    if (ImGui::Selectable(std::string(entities[i]->name + ": " + std::to_string(i)).c_str(), selectedEntityDebug == i))
                        selectedEntityDebug = i;
                }
                ImGui::EndChild();

                ImGui::SameLine();

                if (entitiesSize != 0)
                {
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

                            ImGui::Checkbox("Use Diffuse", &renderer->bUseWholeObjectDiffuseColour);
                            if (renderer->bUseWholeObjectDiffuseColour)
                            {
                                float colors[3] = { renderer->wholeObjectDiffuseRGBA.x, renderer->wholeObjectDiffuseRGBA.y, renderer->wholeObjectDiffuseRGBA.z };
                                ImGui::ColorEdit3("Diffuse", colors);
                                renderer->wholeObjectDiffuseRGBA.x = colors[0];
                                renderer->wholeObjectDiffuseRGBA.y = colors[1];
                                renderer->wholeObjectDiffuseRGBA.z = colors[2];
                            }

                            ImGui::DragFloat("Emmision", &renderer->emmision, 0.1f, 0.0f, 10000.0f);

                            float** tiling = new float* [2];
                            tiling[0] = &renderer->tiling.x;
                            tiling[1] = &renderer->tiling.y;

                            ImGui::DragFloat2("Tiling", *tiling, 0.1f, 0.0f, 10000.0f);

                            delete[] tiling;

                            ImGui::Checkbox("Use Wind", &renderer->useWind);

                            ImGui::EndTabItem();
                        }
                        if (ImGui::BeginTabItem("Details"))
                        {
                            cMeshRenderer* renderer = curEntity->GetComponent<cMeshRenderer>();

                            ImGui::Text(std::string("Mesh Name: " + renderer->meshName).c_str());
                            ImGui::Text(std::string("Friendly Name: " + curEntity->name).c_str());

                            ImGui::EndTabItem();
                        }

                        cInstancedRenderer* instancedRenderer = curEntity->GetComponent<cInstancedRenderer>();
                        if (instancedRenderer != nullptr)
                        {
                            if (ImGui::BeginTabItem("Instanced Renderer"))
                            {
                                if (ImGui::Button("Set Brush to this entity"))
                                {
                                    brush.ChangeRenderer(instancedRenderer);
                                    std::cout << "Set Brush Entity to \"" << curEntity->name << "\"" << std::endl;
                                }
                                ImGui::EndTabItem();
                            }
                        }

                        ImGui::EndTabBar();
                    }

                    ImGui::Separator();
                    if (ImGui::Button("Delete Entity"))
                    {
                        g_entityManager.DeleteEntity(curEntity);
                    }

                    ImGui::EndChild();
                }

                ImGui::EndTabItem();
            }

            if (ImGui::BeginTabItem("Add an Entity"))
            {
                std::vector<sModelDrawInfo> models = gVAOManager.GetLoadedModels();

                if (ImGui::BeginCombo("Model Combo", models[selectedModelDebug].meshName.c_str()))
                {
                    for (size_t i = 0; i < models.size(); i++)
                    {
                        if (ImGui::Selectable(std::string(models[i].meshName + ": " + std::to_string(i)).c_str(), selectedModelDebug == i))
                            selectedModelDebug = i;
                    }
                    ImGui::EndCombo();
                }

                static char nameBuf[64] = ""; ImGui::InputText("Friendly Name", nameBuf, 64);

                ImGui::Separator();
                if(ImGui::Button("Add as Entity"))
                {
                    cEntity* newEntity = g_entityManager.CreateEntity();
                    newEntity->name = nameBuf;

                    cMeshRenderer* newMesh = newEntity->AddComponent<cMeshRenderer>();
                    newMesh->meshName = models[selectedModelDebug].meshName;
                }
                
                ImGui::EndTabItem();
            }
            ImGui::EndTabBar();
        }
        

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
                ImGui::DragFloat("distance cutoff", &curLight->atten.w, 0.5f, 0.0f, 10000.0f);

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

        std::string camPos = "x: " + std::to_string(cameraEye.x) + " y: " + std::to_string(cameraEye.y) + " z: " + std::to_string(cameraEye.z);
        ImGui::Text(camPos.c_str());
        
        ImGui::Text("Mobius Engine by Ethan Robertson");
        ImGui::End();
    }

    //Wind
    {
        ImGui::Begin("Global Wind");
        ImGui::SliderFloat("Wind Strength", &windInfo.strength, 0.0f, 5.0f);
        ImGui::DragFloat("Wind Size", &windInfo.size, 0.01f, 0.0f, 10.0f);

        float directionArray[3] = { windInfo.windDir.x, windInfo.windDir.y, windInfo.windDir.z };
        ImGui::SliderFloat3("Wind Direction", directionArray, -1.0f, 1.0f);
        windInfo.windDir.x = directionArray[0];
        windInfo.windDir.y = directionArray[1];
        windInfo.windDir.z = directionArray[2];


        ImGui::End();
    }

    //ImGui::ShowDemoWindow();

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    ImGui::EndFrame();
}

void SetUpLights()
{
    //OUTSIDE LIGHTS
    gTheLights.theLights[0].name = "Outside light";
    gTheLights.theLights[0].position = glm::vec4(-4.5f, 6.0f, -1.5f, 1.0f);
    gTheLights.theLights[0].diffuse = glm::vec4(1.0f, 0.6f, .05f, 1.0f);
    gTheLights.theLights[0].atten = glm::vec4(0.2f, 0.1f, 0.025f, 100.0f);
    //gTheLights.theLights[0].direction = glm::vec4(0.0f, -1.0f, 1.0f, 1.0f);
    //gTheLights.theLights[0].specular = glm::vec4(1.0f, 1.0f, 1.0f, 50.0f);
    gTheLights.theLights[0].param1.x = 0;
    gTheLights.TurnOnLight(0);  // Or this!
    gTheLights.SetUpUniformLocations(program, 0);

    gTheLights.theLights[1].name = "Outside light";
    gTheLights.theLights[1].position = glm::vec4(3.25f, 5.5f, 8.5f, 1.0f);
    gTheLights.theLights[1].diffuse = glm::vec4(1.0f, 0.6f, .05f, 1.0f);
    gTheLights.theLights[1].atten = glm::vec4(0.2f, 0.1f, 0.025f, 100.0f);
    // gTheLights.theLights[1].direction = glm::vec4(0.0f, -1.0f, 1.0f, 1.0f);
     //gTheLights.theLights[0].specular = glm::vec4(1.0f, 1.0f, 1.0f, 50.0f);
    gTheLights.theLights[1].param1.x = 0;
    gTheLights.TurnOnLight(1);  // Or this!
    gTheLights.SetUpUniformLocations(program, 1);

    gTheLights.theLights[2].name = "Outside light";
    gTheLights.theLights[2].position = glm::vec4(-2.5f, 5.5f, -8.5f, 1.0f);
    gTheLights.theLights[2].diffuse = glm::vec4(1.0f, 0.6f, .05f, 1.0f);
    gTheLights.theLights[2].atten = glm::vec4(0.2f, 0.1f, 0.025f, 100.0f);
    //gTheLights.theLights[2].direction = glm::vec4(0.0f, -1.0f, 1.0f, 1.0f);
    //gTheLights.theLights[0].specular = glm::vec4(1.0f, 1.0f, 1.0f, 50.0f);
    gTheLights.theLights[2].param1.x = 0;
    gTheLights.TurnOnLight(2);  // Or this!
    gTheLights.SetUpUniformLocations(program, 2);

    gTheLights.theLights[3].name = "Outside light";
    gTheLights.theLights[3].position = glm::vec4(6.5f, 5.5f, -8.5f, 1.0f);
    gTheLights.theLights[3].diffuse = glm::vec4(1.0f, 0.6f, .05f, 1.0f);
    gTheLights.theLights[3].atten = glm::vec4(0.2f, 0.1f, 0.025f, 100.0f);
    //gTheLights.theLights[3].direction = glm::vec4(0.0f, -1.0f, 1.0f, 1.0f);
    //gTheLights.theLights[0].specular = glm::vec4(1.0f, 1.0f, 1.0f, 50.0f);
    gTheLights.theLights[3].param1.x = 0;
    gTheLights.TurnOnLight(3);  // Or this!
    gTheLights.SetUpUniformLocations(program, 3);

    gTheLights.theLights[6].name = "Outside light";
    gTheLights.theLights[6].position = glm::vec4(10.5f, 5.5f, -0.25f, 1.0f);
    gTheLights.theLights[6].diffuse = glm::vec4(1.0f, 0.6f, .05f, 1.0f);
    gTheLights.theLights[6].atten = glm::vec4(0.2f, 0.1f, 0.025f, 100.0f);
    //gTheLights.theLights[3].direction = glm::vec4(0.0f, -1.0f, 1.0f, 1.0f);
    //gTheLights.theLights[0].specular = glm::vec4(1.0f, 1.0f, 1.0f, 50.0f);
    gTheLights.theLights[6].param1.x = 0;
    gTheLights.TurnOnLight(6);  // Or this!
    gTheLights.SetUpUniformLocations(program, 6);

    gTheLights.theLights[7].name = "Outside light";
    gTheLights.theLights[7].position = glm::vec4(-10.5f, 6.5f, 4.0f, 1.0f);
    gTheLights.theLights[7].diffuse = glm::vec4(1.0f, 0.6f, .05f, 1.0f);
    gTheLights.theLights[7].atten = glm::vec4(0.2f, 0.1f, 0.025f, 100.0f);
    //gTheLights.theLights[3].direction = glm::vec4(0.0f, -1.0f, 1.0f, 1.0f);
    //gTheLights.theLights[0].specular = glm::vec4(1.0f, 1.0f, 1.0f, 50.0f);
    gTheLights.theLights[7].param1.x = 0;
    gTheLights.TurnOnLight(7);  // Or this!
    gTheLights.SetUpUniformLocations(program, 7);

    gTheLights.theLights[9].name = "Outside light";
    gTheLights.theLights[9].position = glm::vec4(4.25f, 6.0f, 26.5f, 1.0f);
    gTheLights.theLights[9].diffuse = glm::vec4(1.0f, 0.6f, .05f, 1.0f);
    gTheLights.theLights[9].atten = glm::vec4(0.2f, 0.1f, 0.025f, 100.0f);
    //gTheLights.theLights[0].direction = glm::vec4(0.0f, -1.0f, 1.0f, 1.0f);
    //gTheLights.theLights[0].specular = glm::vec4(1.0f, 1.0f, 1.0f, 50.0f);
    gTheLights.theLights[9].param1.x = 0;
    gTheLights.TurnOnLight(9);  // Or this!
    gTheLights.SetUpUniformLocations(program, 9);


    //INSIDE LIGHTS
    gTheLights.theLights[4].name = "Inside light";
    gTheLights.theLights[4].position = glm::vec4(1.5f, 7.5f, -0.25f, 1.0f);
    gTheLights.theLights[4].diffuse = glm::vec4(0.76f, 0.9f, 1.0f, 1.0f);
    gTheLights.theLights[4].atten = glm::vec4(0.2f, 0.1f, 0.005f, 100.0f);
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
    gTheLights.theLights[5].atten = glm::vec4(0.2f, 0.1f, 0.005f, 100.0f);
    gTheLights.theLights[5].direction = glm::vec4(0.0f, -1.0f, 0.0f, 1.0f);
    //gTheLights.theLights[0].specular = glm::vec4(1.0f, 1.0f, 1.0f, 50.0f);
    gTheLights.theLights[5].param1.x = 1;
    gTheLights.theLights[5].param1.y = 20.0f;
    gTheLights.theLights[5].param1.z = 25.0f;
    gTheLights.TurnOnLight(5);  // Or this!
    gTheLights.SetUpUniformLocations(program, 5);

    //SUN

    gTheLights.theLights[8].name = "Sun light";
    gTheLights.theLights[8].position = glm::vec4(-2.f, 30.f, 17.f, 1.0f);
    gTheLights.theLights[8].diffuse = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
    gTheLights.theLights[8].atten = glm::vec4(0.2f, 0.1f, 0.005f, 100.0f);
    gTheLights.theLights[8].direction = glm::normalize(glm::vec4(0.2f, -.8f, -.4f, 1.0f));
    //gTheLights.theLights[0].specular = glm::vec4(1.0f, 1.0f, 1.0f, 50.0f);
    gTheLights.theLights[8].param1.x = 2;

    gTheLights.theLights[8].power = 2.36;
    gTheLights.TurnOnLight(8);  // Or this!
    gTheLights.SetUpUniformLocations(program, 8);

    gTheLights.theLights[12].position = glm::vec4(0.f, 0.f, 0.f, 1.0f);
    gTheLights.theLights[12].diffuse = glm::vec4(.5f, 0.5f, .5f, 1.0f);
    gTheLights.theLights[12].atten = glm::vec4(0.2f, 0.1f, 0.005f, 100.0f);
    gTheLights.theLights[12].direction = glm::vec4(0.0f, .9f, .4f, 1.0f);
    gTheLights.theLights[12].param1.x = 2;
    gTheLights.TurnOffLight(12);  // Or this!
    gTheLights.SetUpUniformLocations(program, 12);
}