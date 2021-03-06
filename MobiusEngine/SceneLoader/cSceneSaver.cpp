#include "cSceneLoader.h"
#include <fstream>
#include <iostream>

#include <rapidxml/rapidxml.hpp>
#include <rapidxml/rapidxml_print.hpp>
#include "../Components/cMeshRenderer.h"
#include "../Components/cTransform.h"
#include "../Components/cInstancedRenderer.h"

bool cSceneLoader::SaveScene(std::string sceneName, glm::vec3 cameraPos, cEntityManager* entityManager)
{
	using namespace rapidxml;
	xml_document<>* doc = new rapidxml::xml_document<>;
	xml_node<>* sceneNode = doc->allocate_node(rapidxml::node_element, "Scene");

	//Camera Position	
	xml_node<>* cameraNode = doc->allocate_node(rapidxml::node_element, "Camera");
	xml_node<>* cameraTransformNode = doc->allocate_node(rapidxml::node_element, "Transform");
	xml_node<>* cameraPositionNode = doc->allocate_node(rapidxml::node_element, "Position");

	char* xCamString = doc->allocate_string(std::to_string(cameraPos.x).c_str());
	char* yCamString = doc->allocate_string(std::to_string(cameraPos.y).c_str());
	char* zCamString = doc->allocate_string(std::to_string(cameraPos.z).c_str());

	cameraPositionNode->append_attribute(doc->allocate_attribute("x", xCamString));
	cameraPositionNode->append_attribute(doc->allocate_attribute("y", yCamString));
	cameraPositionNode->append_attribute(doc->allocate_attribute("z", zCamString));
	cameraTransformNode->append_node(cameraPositionNode);
	cameraNode->append_node(cameraTransformNode);
	sceneNode->append_node(cameraNode);

	xml_node<>* modelsNode = doc->allocate_node(rapidxml::node_element, "Models");
	for (sModel model : models)
	{
		xml_node<>* modelNode = doc->allocate_node(rapidxml::node_element, "Model");
		modelNode->value(doc->allocate_string(model.fileName.c_str()));
		modelsNode->append_node(modelNode);
	}
	sceneNode->append_node(modelsNode);

	xml_node<>* descNode = doc->allocate_node(rapidxml::node_element, "Description");

	//Includes entity childen to be part of the save file
	std::vector<cEntity*> finalEntityList;
	for (cEntity* entity : entityManager->GetEntities())
	{
		finalEntityList.push_back(entity);
		for (cEntity* child : entity->children)
		{
			finalEntityList.push_back(child);
		}
	}

	for (cEntity* entity : finalEntityList)
	{
		if (entity->isGameplayEntity)
			continue;

		cMeshRenderer* mesh = entity->GetComponent<cMeshRenderer>();
		cTransform* meshTransform = entity->GetComponent<cTransform>();

		
		xml_node<>* meshNode = doc->allocate_node(rapidxml::node_element, "Mesh");
		meshNode->append_attribute(doc->allocate_attribute("FriendlyName", doc->allocate_string(entity->name.c_str())));
		meshNode->append_attribute(doc->allocate_attribute("MeshName", doc->allocate_string(mesh->meshName.c_str())));

		if (entity->childOf != "")
		{
			meshNode->append_attribute(doc->allocate_attribute("ChildOf", doc->allocate_string(entity->childOf.c_str())));
		}

		//Transform		
		xml_node<>* transformNode = doc->allocate_node(rapidxml::node_element, "Transform");
		xml_node<>* posNode = doc->allocate_node(rapidxml::node_element, "Position");
		
		char* xPosString = doc->allocate_string(std::to_string(meshTransform->position.x).c_str());
		char* yPosString = doc->allocate_string(std::to_string(meshTransform->position.y).c_str());
		char* zPosString = doc->allocate_string(std::to_string(meshTransform->position.z).c_str());
		
		posNode->append_attribute(doc->allocate_attribute("x", xPosString));
		posNode->append_attribute(doc->allocate_attribute("y", yPosString));
		posNode->append_attribute(doc->allocate_attribute("z", zPosString));
		
		xml_node<>* rotNode = doc->allocate_node(rapidxml::node_element, "Rotation");
		
		glm::vec3 euler = meshTransform->GetEulerRotation();
		char* xRotString = doc->allocate_string(std::to_string(glm::degrees(euler.x)).c_str());
		char* yRotString = doc->allocate_string(std::to_string(glm::degrees(euler.y)).c_str());
		char* zRotString = doc->allocate_string(std::to_string(glm::degrees(euler.z)).c_str());
		
		rotNode->append_attribute(doc->allocate_attribute("x", xRotString));
		rotNode->append_attribute(doc->allocate_attribute("y", yRotString));
		rotNode->append_attribute(doc->allocate_attribute("z", zRotString));
		
		xml_node<>* scaleNode = doc->allocate_node(rapidxml::node_element, "Scale");
		
		char* xScaleString = doc->allocate_string(std::to_string(meshTransform->scale.x).c_str());
		char* yScaleString = doc->allocate_string(std::to_string(meshTransform->scale.y).c_str());
		char* zScaleString = doc->allocate_string(std::to_string(meshTransform->scale.z).c_str());
		
		scaleNode->append_attribute(doc->allocate_attribute("x", xScaleString));
		scaleNode->append_attribute(doc->allocate_attribute("y", yScaleString));
		scaleNode->append_attribute(doc->allocate_attribute("z", zScaleString));
		
		transformNode->append_node(posNode);
		transformNode->append_node(rotNode);
		transformNode->append_node(scaleNode);
		meshNode->append_node(transformNode);
		
		//Colors
		if (mesh->bUseWholeObjectDiffuseColour)
		{
			xml_node<>* colorsNode = doc->allocate_node(rapidxml::node_element, "Colors");
			xml_node<>* overrideNode = doc->allocate_node(rapidxml::node_element, "OverrideColor");
		
			std::string overrideString = mesh->bUseWholeObjectDiffuseColour == true ? "true" : "false";
			overrideNode->value(doc->allocate_string(overrideString.c_str()), overrideString.size());
		
			std::string rString = std::to_string(mesh->wholeObjectDiffuseRGBA.r);
			std::string gString = std::to_string(mesh->wholeObjectDiffuseRGBA.g);
			std::string bString = std::to_string(mesh->wholeObjectDiffuseRGBA.b);
			std::string aString = std::to_string(mesh->wholeObjectDiffuseRGBA.a);
		
			char* r = doc->allocate_string(rString.c_str());
			char* g = doc->allocate_string(gString.c_str());
			char* b = doc->allocate_string(bString.c_str());
			char* a = doc->allocate_string(aString.c_str());
		
			xml_node<>* colorNode = doc->allocate_node(rapidxml::node_element, "Color");
			colorNode->append_attribute(doc->allocate_attribute("r", r));
			colorNode->append_attribute(doc->allocate_attribute("g", g));
			colorNode->append_attribute(doc->allocate_attribute("b", b));
			colorNode->append_attribute(doc->allocate_attribute("a", a));
		
			colorsNode->append_node(overrideNode);
			colorsNode->append_node(colorNode);
		
			meshNode->append_node(colorsNode);
		}
		
		
		xml_node<>* miscNode = doc->allocate_node(rapidxml::node_element, "Misc");
		xml_node<>* lightingNode = doc->allocate_node(rapidxml::node_element, "UseLighting");
		
		std::string lightingString = mesh->bDontLight == true ? "false" : "true";
		lightingNode->value(doc->allocate_string(lightingString.c_str()));
		miscNode->append_node(lightingNode);
		
		if (mesh->useAlbedoMap)
		{
			xml_node<>* texturesNode = doc->allocate_node(rapidxml::node_element, "Textures");
		
			for (Texture t : mesh->textures)
			{
				xml_node<>* textureNode = doc->allocate_node(rapidxml::node_element, "Texture");
				textureNode->value(doc->allocate_string(t.name.c_str()));
		
				std::string ratioString = std::to_string(1.0f);
				char* ratioCharArray = doc->allocate_string(ratioString.c_str());
				textureNode->append_attribute(doc->allocate_attribute("Ratio", ratioCharArray));
		
				if (mesh->bUseSkybox)
				{
					char* skyboxArray = doc->allocate_string("SkyBox");
					textureNode->append_attribute(doc->allocate_attribute("Type", skyboxArray));
				}
		
				texturesNode->append_node(textureNode);
			}
			miscNode->append_node(texturesNode);
		}
		
		{
			xml_node<>* metallicNode = doc->allocate_node(rapidxml::node_element, "Metallic");
			metallicNode->value(doc->allocate_string(std::to_string(mesh->metallic).c_str()));
		
			miscNode->append_node(metallicNode);
		}
		
		if (mesh->useMetallicMap)
		{
			xml_node<>* metallicMapNode = doc->allocate_node(rapidxml::node_element, "MetallicMap");
			metallicMapNode->value(doc->allocate_string(mesh->metallicMapName.c_str()));
		
			miscNode->append_node(metallicMapNode);
		}
		
		{
			xml_node<>* roughNode = doc->allocate_node(rapidxml::node_element, "Roughness");
			roughNode->value(doc->allocate_string(std::to_string(mesh->roughness).c_str()));
		
			miscNode->append_node(roughNode);
		}
		
		if (mesh->useRoughnessMap)
		{
			xml_node<>* roughMapNode = doc->allocate_node(rapidxml::node_element, "RoughnessMap");
			roughMapNode->value(doc->allocate_string(mesh->roughnessMapName.c_str()));
		
			miscNode->append_node(roughMapNode);
		}
		
		if (mesh->bUseAlphaMask)
		{
			xml_node<>* alphaMaskNode = doc->allocate_node(rapidxml::node_element, "AlphaMask");
			alphaMaskNode->value(doc->allocate_string(mesh->alphaMaskName.c_str()));
		
			miscNode->append_node(alphaMaskNode);
		}
		
		if (mesh->bUseNormalMap)
		{
			xml_node<>* normalMapNode = doc->allocate_node(rapidxml::node_element, "NormalMap");
			normalMapNode->value(doc->allocate_string(mesh->normalMapName.c_str()));
		
			miscNode->append_node(normalMapNode);
		}

		if (mesh->useAOMap)
		{
			xml_node<>* aoMapNode = doc->allocate_node(rapidxml::node_element, "AOMap");
			aoMapNode->value(doc->allocate_string(mesh->AOMapName.c_str()));

			miscNode->append_node(aoMapNode);
		}
		
		if (mesh->bUseHeightMap)
		{
			xml_node<>* heightMapNode = doc->allocate_node(rapidxml::node_element, "HeightMap");
			heightMapNode->value(doc->allocate_string(mesh->heightMapName.c_str()));
		
			miscNode->append_node(heightMapNode);
		}
		
		if (mesh->tiling.x != 0.0f || mesh->tiling.y != 0.0f || mesh->offset.x != 0.0f || mesh->offset.y != 0.0f)
		{
			xml_node<>* tilingOffsetNode = doc->allocate_node(rapidxml::node_element, "TilingOffset");

			std::string tilingXString = std::to_string(mesh->tiling.x);
			std::string tilingYString = std::to_string(mesh->tiling.y);
			std::string offsetXString = std::to_string(mesh->offset.x);
			std::string offsetYString = std::to_string(mesh->offset.y);

			char* tilingX = doc->allocate_string(tilingXString.c_str());
			char* tilingY = doc->allocate_string(tilingYString.c_str());
			char* offsetX = doc->allocate_string(offsetXString.c_str());
			char* offsetY = doc->allocate_string(offsetYString.c_str());

			tilingOffsetNode->append_attribute(doc->allocate_attribute("TilingX", tilingX));
			tilingOffsetNode->append_attribute(doc->allocate_attribute("TilingY", tilingY));
			tilingOffsetNode->append_attribute(doc->allocate_attribute("OffsetX", offsetX));
			tilingOffsetNode->append_attribute(doc->allocate_attribute("OffsetY", offsetY));

			miscNode->append_node(tilingOffsetNode);
		}
		
		if (mesh->emmisionPower != 0.0f) 
		{
			xml_node<>* emmisionNode = doc->allocate_node(rapidxml::node_element, "Emmision");
			std::string emmisionString = std::to_string(mesh->emmisionPower);
			emmisionNode->value(doc->allocate_string(emmisionString.c_str()));
		
			std::string rString = std::to_string(mesh->emmisionDiffuse.r);
			std::string gString = std::to_string(mesh->emmisionDiffuse.g);
			std::string bString = std::to_string(mesh->emmisionDiffuse.b);
		
			char* r = doc->allocate_string(rString.c_str());
			char* g = doc->allocate_string(gString.c_str());
			char* b = doc->allocate_string(bString.c_str());
		
			emmisionNode->append_attribute(doc->allocate_attribute("r", r));
			emmisionNode->append_attribute(doc->allocate_attribute("g", g));
			emmisionNode->append_attribute(doc->allocate_attribute("b", b));
		
			miscNode->append_node(emmisionNode);
		}
		
		if (mesh->shadowBias != 0.005f)
		{
			xml_node<>* biasNode = doc->allocate_node(rapidxml::node_element, "ShadowBias");
			std::string biasString = std::to_string(mesh->shadowBias);
			biasNode->value(doc->allocate_string(biasString.c_str()));
		
			miscNode->append_node(biasNode);
		}
		
		if (mesh->diffuseBrightness != 1.0f)
		{
			xml_node<>* brightnessNode = doc->allocate_node(rapidxml::node_element, "Brightness");
			std::string biasString = std::to_string(mesh->diffuseBrightness);
			brightnessNode->value(doc->allocate_string(biasString.c_str()));
		
			miscNode->append_node(brightnessNode);
		}
		
		cInstancedRenderer* instancedRenderer = entity->GetComponent<cInstancedRenderer>();
		if (instancedRenderer != nullptr)
		{
			xml_node<>* instancedNode = doc->allocate_node(rapidxml::node_element, "Instanced");
			
			if (instancedRenderer->UseFile())
			{
				instancedNode->value(doc->allocate_string(instancedRenderer->fileName.c_str()));
			}

			std::string amountString = std::to_string(instancedRenderer->GetCount());
			char* amountCharArray = doc->allocate_string(amountString.c_str());
			instancedNode->append_attribute(doc->allocate_attribute("Amount", amountCharArray));
		
			std::string offsetString = std::to_string(instancedRenderer->GetOffset());
			char* offsetCharArray = doc->allocate_string(offsetString.c_str());
			instancedNode->append_attribute(doc->allocate_attribute("Offset", offsetCharArray));
		
			std::string randomString = std::to_string(instancedRenderer->GetRandomStrength());
			char* randomCharArray = doc->allocate_string(randomString.c_str());
			instancedNode->append_attribute(doc->allocate_attribute("RandomStrength", randomCharArray));
		
			miscNode->append_node(instancedNode);
		}
		
		if (mesh->useWind)
		{
			xml_node<>* windNode = doc->allocate_node(rapidxml::node_element, "UseWind");
			miscNode->append_node(windNode);
		}
		
		meshNode->append_node(miscNode);
		descNode->append_node(meshNode);
		
	}

	sceneNode->append_node(descNode);
	doc->append_node(sceneNode);

	std::ofstream sceneFile("assets/scenes/" + sceneName + ".xml");
	if (!sceneFile.is_open())
	{
		std::cout << "ERROR could not open sceneFile file from scene saver" << std::endl;
		return false;
	}
	else
	{
		sceneFile << *doc;

		sceneFile.close();

		doc->clear();
		delete doc;

		std::cout << "Saved scene!" << std::endl;

		return true;
	}	
}