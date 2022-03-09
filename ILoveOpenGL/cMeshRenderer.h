#pragma once
#include "cComponent.h"

#include <string>
#include <glm/glm.hpp>
#include <glm/vec3.hpp>
#include <vector>
#include "cEntity.h"

struct Texture
{
	std::string name;
	float ratio;
};

class cMeshRenderer : public cComponent
{
public:

	cMeshRenderer();

	virtual void Update(float dt) {};

	std::string meshName;		// The 3D Mesh model we are drawing

	// Use these values to search for this specific instance of the object
	// We would set these ourselves...
	std::string friendlyName;
	unsigned int friendlyID;

	// Sets the overall colour of the object
	//	by overwriting the colour variable
	// HACK: We'll get rid of this once we have textures
	glm::vec4 objectDebugColourRGBA;
	bool bUseObjectDebugColour;
	bool bDontLight;
	// Changes polygon mode to LINES (instead of solid)
	bool bIsWireframe;
	// Turns of the depth buffer check when drawn
	bool bDisableDepthBufferCheck;

	bool bIsDebugObject;
	// These are for colouring the ENTIRE object as one colour.
	// Later, these will be replaced:
	// * The diffuse will be replaced by textures
	// * The specular will be replaced by specular and gloss maps
	glm::vec4 wholeObjectDiffuseRGBA;		// The "colour" of the object
	bool bUseWholeObjectDiffuseColour;		// If true, then wholeObject colour (not model vertex) is used

	glm::vec3 wholeObjectSpecularRGB;		// Specular HIGHLIGHT colour (usually the same as the light, or white)
	float wholeObjectShininess_SpecPower;	// 1.0 to ??

	bool bUseSpecular;
	//float alphaTransparency;

	bool bUseAlphaMask;
	bool bUseNormalMap;
	bool bUseHeightMap;

	bool bUseSkybox;
	bool bUseSkyboxReflection;
	bool bUseSkyboxRefraction;

	bool bIsImposter = false;

	std::vector<Texture> textures;
	std::string alphaMaskName;
	std::string normalMapName;
	std::string heightMapName;

	glm::vec2 normalOffset;

	const unsigned int MAX_TEXTURES = 4;

	bool bIsSceneObject;
};