#pragma once
#include "cComponent.h"

#include <string>
#include <glm/glm.hpp>
#include <glm/vec3.hpp>
#include <vector>
#include "../cEntity.h"

struct Texture
{
	std::string name;
	float ratio = 1.0f;
};

//Holds the mesh info related to an entity
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
	// These are for colouring the ENTIRE object as one colour.
	// Later, these will be replaced:
	// * The diffuse will be replaced by textures
	// * The specular will be replaced by specular and gloss maps
	glm::vec4 wholeObjectDiffuseRGBA;		// The "colour" of the object
	bool bUseWholeObjectDiffuseColour;		// If true, then wholeObject colour (not model vertex) is used

	float diffuseBrightness;

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

	float emmisionPower;
	glm::vec3 emmisionDiffuse;
	float shadowBias;

	float roughness;
	bool useRoughnessMap;
	std::string roughnessMapName;

	float metallic;
	bool useMetallicMap;
	std::string metallicMapName;

	bool useAOMap;
	std::string AOMapName;

	bool bIsImposter = false;
	bool useWind = false;

	bool render = true;

	bool useAlbedoMap = false;
	std::vector<Texture> textures;
	glm::vec2 tiling;
	glm::vec2 offset;

	std::string alphaMaskName;
	std::string normalMapName;
	std::string heightMapName;

	const unsigned int MAX_TEXTURES = 1;
};