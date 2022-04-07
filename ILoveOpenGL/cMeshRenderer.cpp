#include "cMeshRenderer.h"

cMeshRenderer::cMeshRenderer()
{

	this->bIsWireframe = false;
	this->bDisableDepthBufferCheck = false;

	this->objectDebugColourRGBA = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);	// White
	this->bUseObjectDebugColour = false;
	this->bDontLight = false;

	this->bIsDebugObject = false;
	this->emmisionPower = 0.0f;
	this->emmisionDiffuse = glm::vec3(1.0f);
	this->shadowBias = 0.005f;

	this->diffuseBrightness = 1.0f;

	this->roughness = 1.0f;
	this->metallic = 0.0f;

	this->tiling = glm::vec2(1.0f);
	this->offset = glm::vec2(0.0f);

	// These are for colouring the ENTIRE object as one colour.
	// Later, these will be replaced:
	// * The diffuse will be replaced by textures
	// * The specular will be replaced by specular and gloss maps
	this->wholeObjectDiffuseRGBA = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);	// The "colour" of the object
	this->bUseWholeObjectDiffuseColour = false;
	// Specular HIGHLIGHT colour (usually the same as the light, or white)
	this->wholeObjectSpecularRGB = glm::vec3(1.0f, 1.0f, 1.0f);
	// Specular highlight POWER (or shininess). Starts at 1.0 and goes to ? (like 100, 1000, 10000 is OK)
	this->wholeObjectShininess_SpecPower = 250.0f;	// 1.0 to ??

	//this->alphaTransparency = 1.0f;

	for (unsigned int i = 0; i < MAX_TEXTURES; i++)
	{
		Texture newTexture;
		newTexture.ratio = 0.0f;
		this->textures.push_back(newTexture);
	}

	this->bUseAlphaMask = false;
	this->bUseSpecular = false;
	this->bUseSkybox = false;
	this->bUseNormalMap = false;
	this->bUseHeightMap = false;
	this->normalOffset = glm::vec2(0.0f);

	bIsSceneObject = false;
	bUseSkyboxReflection = false;
	bUseSkyboxRefraction = false;
}