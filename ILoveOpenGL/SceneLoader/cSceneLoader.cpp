#include "cSceneLoader.h"

#include <fstream>
#include <iostream>
#include <rapidxml/rapidxml.hpp>
#include <rapidxml/rapidxml_utils.hpp>
#include "../cMeshRenderer.h"
#include "../cTransform.h"
#include "../cTextureViewer.h"
#include "../cInstancedRenderer.h"

cSceneLoader::cSceneLoader()
{
	loadedModels = false;
	cameraPosition = glm::vec3(0);
}

cSceneLoader::~cSceneLoader()
{
}

cSceneLoader* cSceneLoader::GetSceneLoaderInstance()
{
	return &cSceneLoader::_instance;
}
cSceneLoader cSceneLoader::_instance;


bool cSceneLoader::LoadScene(std::string sceneName, cBasicTextureManager* textureManager, cEntityManager* entityManager)
{
	using namespace rapidxml;
	bool useFBOTexture = false;

	std::string sceneFile("assets/scenes/" + sceneName + ".xml");

	rapidxml::file<>* xmlFile = new file<>(sceneFile.c_str());
	xml_document<>* doc = new xml_document<>;
	doc->parse<0>(xmlFile->data());

	xml_node<>* sceneNode = doc->first_node("Scene");
	xml_node<>* cameraPosNode = sceneNode->first_node("Camera")->first_node("Transform")->first_node("Position");
	float camX = std::stof(cameraPosNode->first_attribute("x")->value());
	float camY = std::stof(cameraPosNode->first_attribute("y")->value());
	float camZ = std::stof(cameraPosNode->first_attribute("z")->value());
	cameraPosition = glm::vec3(camX, camY, camZ);

	rapidxml::xml_node<>* modelsNode = sceneNode->first_node("Models");
	for (rapidxml::xml_node<>* modelNode = modelsNode->first_node(); modelNode != NULL; modelNode = modelNode->next_sibling())
	{
		sModel newModel;
		newModel.fileName = modelNode->value();

		models.push_back(newModel);
	}

	rapidxml::xml_node<>* descNode = sceneNode->first_node("Description");
	for (rapidxml::xml_node<>* meshNode = descNode->first_node(); meshNode != NULL; meshNode = meshNode->next_sibling())
	{
		cEntity* newEntity;

		xml_attribute<>* childAttrib = meshNode->first_attribute("ChildOf");
		if (childAttrib != nullptr)
		{
			std::string parentName = childAttrib->value();
			newEntity = entityManager->CreateEntity(false);

			entityManager->GetEntityByName(parentName)->children.push_back(newEntity);

			newEntity->childOf = parentName;
		}
		else
		{
			newEntity = entityManager->CreateEntity();
		}

		cMeshRenderer* newMesh = new cMeshRenderer();
		cTransform newTransform;

		newMesh->friendlyName = meshNode->first_attribute("FriendlyName")->value();
		newMesh->meshName = meshNode->first_attribute("MeshName")->value();

		xml_node<>* transformNode = meshNode->first_node("Transform");

		glm::vec3 pos;
		pos.x = std::stof(transformNode->first_node("Position")->first_attribute("x")->value());
		pos.y = std::stof(transformNode->first_node("Position")->first_attribute("y")->value());
		pos.z = std::stof(transformNode->first_node("Position")->first_attribute("z")->value());
		newTransform.position = pos;

		glm::vec3 rot;
		rot.x = glm::radians(std::stof(transformNode->first_node("Rotation")->first_attribute("x")->value()));
		rot.y = glm::radians(std::stof(transformNode->first_node("Rotation")->first_attribute("y")->value()));
		rot.z = glm::radians(std::stof(transformNode->first_node("Rotation")->first_attribute("z")->value()));
		newTransform.SetRotation(rot);

		xml_node<>* scaleNode = transformNode->first_node("Scale");
		std::string scaleString = scaleNode->value();

		if (scaleString != "")
		{
			newTransform.scale = glm::vec3(std::stof(scaleString));
		}
		else
		{
			glm::vec3 scale;
			scale.x = std::stof(scaleNode->first_attribute("x")->value());
			scale.y = std::stof(scaleNode->first_attribute("y")->value());
			scale.z = std::stof(scaleNode->first_attribute("z")->value());
			newTransform.scale = scale;
		}
		
		xml_node<>* colorNode = meshNode->first_node("Colors");
		if (colorNode != nullptr)
		{

			xml_node<>* overrideColorNode = colorNode->first_node("OverrideColor");
			if (overrideColorNode != nullptr)
			{
				std::string useColor = overrideColorNode->value();
				newMesh->bUseWholeObjectDiffuseColour = useColor == "true" ? true : false;
			}

			glm::vec4 colors;
			colors.r = std::stof(colorNode->first_node("Color")->first_attribute("r")->value());
			colors.g = std::stof(colorNode->first_node("Color")->first_attribute("g")->value());
			colors.b = std::stof(colorNode->first_node("Color")->first_attribute("b")->value());
			colors.a = std::stof(colorNode->first_node("Color")->first_attribute("a")->value());
			newMesh->wholeObjectDiffuseRGBA = colors;
		}
		xml_node<>* miscNode = meshNode->first_node("Misc");

		if (miscNode != nullptr)
		{
			std::string useLighting = miscNode->first_node("UseLighting")->value();
			newMesh->bDontLight = useLighting == "true" ? false : true;

			xml_node<>* texturesNode = miscNode->first_node("Textures");
			if (texturesNode != nullptr)
			{
				unsigned int index = 0;
				for (rapidxml::xml_node<>* textureNode = texturesNode->first_node(); textureNode != NULL && index < newMesh->MAX_TEXTURES; textureNode = textureNode->next_sibling())
				{
					Texture newTexture;
					newTexture.name = textureNode->value();
					if (newTexture.name != "")
					{
						newMesh->useAlbedoMap = true;
					}

					newTexture.ratio = std::stof(textureNode->first_attribute("Ratio")->value());

					if (textureNode->first_attribute("Type") != nullptr)
					{
						std::string textureType = textureNode->first_attribute("Type")->value();

						if(textureType == "SkyBox")
							newMesh->bUseSkybox = true;

						if (textureType == "FBO")
						{
							useFBOTexture = true;
							continue;
						}
					}

					newMesh->textures[index] = newTexture;

					if (!newMesh->bUseSkybox && textureManager->getTextureIDFromName(newTexture.name) == 0)
					{
						if (newTexture.name != "" && !textureManager->Create2DTextureFromBMPFile(newTexture.name, true))
						{
							std::cout << "ERROR! Could not add texture to manager: "<< newTexture.name << std::endl;
						}
					}

					index++;
				}
			}
			else
			{
				newMesh->useAlbedoMap = false;
			}

			xml_node<>* alphaMaskNode = miscNode->first_node("AlphaMask");
			if (alphaMaskNode != nullptr)
			{
				newMesh->bUseAlphaMask = true;
				newMesh->alphaMaskName = alphaMaskNode->value();

				if (textureManager->getTextureIDFromName(newMesh->alphaMaskName) == 0)
				{
					if (!textureManager->Create2DTextureFromBMPFile(newMesh->alphaMaskName, true))
					{
						std::cout << "ERROR! Could not add alpha mask to manager: " << newMesh->alphaMaskName << std::endl;
					}
				}
			}

			xml_node<>* normalMapNode = miscNode->first_node("NormalMap");
			if (normalMapNode != nullptr)
			{
				newMesh->bUseNormalMap = true;
				newMesh->normalMapName = normalMapNode->value();

				if (textureManager->getTextureIDFromName(newMesh->normalMapName) == 0)
				{
					if (!textureManager->Create2DTextureFromBMPFile(newMesh->normalMapName, true))
					{
						std::cout << "ERROR! Could not add normal map to manager: " << newMesh->normalMapName << std::endl;
					}
				}
			}

			xml_node<>* heightMapNode = miscNode->first_node("HeightMap");
			if (heightMapNode != nullptr)
			{
				newMesh->bUseHeightMap = true;
				newMesh->heightMapName = heightMapNode->value();

				if (textureManager->getTextureIDFromName(newMesh->heightMapName) == 0)
				{
					if (!textureManager->Create2DTextureFromBMPFile(newMesh->heightMapName, true))
					{
						std::cout << "ERROR! Could not add height map to manager: " << newMesh->heightMapName << std::endl;
					}
				}
			}

			xml_node<>* metallicNode = miscNode->first_node("Metallic");
			if (metallicNode != nullptr)
			{
				newMesh->metallic = std::stof(metallicNode->value());
			}
			xml_node<>* metallicMapNode = miscNode->first_node("MetallicMap");
			if (metallicMapNode != nullptr)
			{
				newMesh->useMetallicMap = true;
				newMesh->metallicMapName = metallicMapNode->value();

				if (textureManager->getTextureIDFromName(newMesh->metallicMapName) == 0)
				{
					if (!textureManager->Create2DTextureFromBMPFile(newMesh->metallicMapName, true))
					{
						std::cout << "ERROR! Could not add metallic map to manager: " << newMesh->metallicMapName << std::endl;
					}
				}
			}

			xml_node<>* roughNode = miscNode->first_node("Roughness");
			if (roughNode != nullptr)
			{
				newMesh->roughness = std::stof(roughNode->value());
			}
			xml_node<>* roughMapNode = miscNode->first_node("RoughnessMap");
			if (roughMapNode != nullptr)
			{
				newMesh->useRoughnessMap = true;
				newMesh->roughnessMapName = roughMapNode->value();

				if (textureManager->getTextureIDFromName(newMesh->roughnessMapName) == 0)
				{
					if (!textureManager->Create2DTextureFromBMPFile(newMesh->roughnessMapName, true))
					{
						std::cout << "ERROR! Could not add roughness map to manager: " << newMesh->roughnessMapName << std::endl;
					}
				}
			}

			xml_node<>* aoMapNode = miscNode->first_node("AOMap");
			if (aoMapNode != nullptr)
			{
				newMesh->useAOMap = true;
				newMesh->AOMapName = aoMapNode->value();

				if (textureManager->getTextureIDFromName(newMesh->AOMapName) == 0)
				{
					if (!textureManager->Create2DTextureFromBMPFile(newMesh->AOMapName, true))
					{
						std::cout << "ERROR! Could not add AO map to manager: " << newMesh->AOMapName << std::endl;
					}
				}
			}

			xml_node<>* tilingOffsetNode = miscNode->first_node("TilingOffset");
			if(tilingOffsetNode != nullptr)
			{
				newMesh->tiling.x = std::stof(tilingOffsetNode->first_attribute("TilingX")->value());
				newMesh->tiling.y = std::stof(tilingOffsetNode->first_attribute("TilingY")->value());

				newMesh->offset.x = std::stof(tilingOffsetNode->first_attribute("OffsetX")->value());
				newMesh->offset.y = std::stof(tilingOffsetNode->first_attribute("OffsetY")->value());
			}

			xml_node<>* emmisionNode = miscNode->first_node("Emmision");
			if (emmisionNode != nullptr)
			{
				float emmision = std::stof(emmisionNode->value());
				newMesh->emmisionPower = emmision;

				glm::vec3 colors;
				colors.r = std::stof(emmisionNode->first_attribute("r")->value());
				colors.g = std::stof(emmisionNode->first_attribute("g")->value());
				colors.b = std::stof(emmisionNode->first_attribute("b")->value());

				newMesh->emmisionDiffuse = colors;
			}
			xml_node<>* shadowBiasNode = miscNode->first_node("ShadowBias");
			if (shadowBiasNode != nullptr)
			{
				float bias = std::stof(shadowBiasNode->value());
				newMesh->shadowBias = bias;
			}

			xml_node<>* brightnessNode = miscNode->first_node("Brightness");
			if (brightnessNode != nullptr)
			{
				float brightness = std::stof(brightnessNode->value());
				newMesh->diffuseBrightness = brightness;
			}

			xml_node<>* treeNode = miscNode->first_node("Instanced");
			if (treeNode != nullptr)
			{
				std::string fileName = treeNode->value();
				unsigned int amount = std::stoi(treeNode->first_attribute("Amount")->value());
				float offset = std::stof(treeNode->first_attribute("Offset")->value());
				float randomStrength = std::stof(treeNode->first_attribute("RandomStrength")->value());

				cInstancedRenderer* instancedRenderer = new cInstancedRenderer(amount, offset, fileName, randomStrength);
				newEntity->AddComponent<cInstancedRenderer>(instancedRenderer);
			}

			xml_node<>* windNode = miscNode->first_node("UseWind");
			newMesh->useWind = windNode != nullptr;
		}

		newEntity->name = newMesh->friendlyName;
		newEntity->AddComponent<cMeshRenderer>(newMesh);
		*newEntity->GetComponent<cTransform>() = newTransform;

		if (useFBOTexture)
		{
			newEntity->AddComponent<cTextureViewer>();
		}
	}

	delete xmlFile;
	doc->clear();
	delete doc;
	return true;
}

bool cSceneLoader::LoadIntoVAO(cVAOManager* vao, GLuint program)
{
	bool loadedProperly = true;

	for (int i = 0; i < this->models.size(); i++)
	{
		sModelDrawInfo modelInfo;
		if (vao->LoadModelIntoVAO(models[i].fileName, modelInfo, program))
		{
			//std::cout << "\tLoaded model: " << models[i].fileName << std::endl;
		}
		else
		{
			loadedProperly = false;
			std::cout << "\tERROR: issue loading '" << models[i].fileName << "' into VAO" << std::endl;
		}
	}	

	return loadedProperly;
}

std::vector<sModel>* cSceneLoader::GetModels()
{
	return &this->models;
}

glm::vec3 cSceneLoader::GetCameraStartingPosition()
{
	return cameraPosition;
}

