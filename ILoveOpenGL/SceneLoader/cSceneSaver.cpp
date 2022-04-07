#include "cSceneLoader.h"
#include <fstream>
#include <iostream>

#include <rapidxml/rapidxml.hpp>
#include <rapidxml/rapidxml_print.hpp>
#include "../cMeshRenderer.h"
#include "../cTransform.h"
#include "../cInstancedRenderer.h"

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

	for (cEntity* entity : entityManager->GetEntities())
	{
		cMeshRenderer* mesh = entity->GetComponent<cMeshRenderer>();
		cTransform* meshTransform = entity->GetComponent<cTransform>();

		if (!mesh->bIsDebugObject && mesh->bIsSceneObject)
		{
			xml_node<>* meshNode = doc->allocate_node(rapidxml::node_element, "Mesh");
			meshNode->append_attribute(doc->allocate_attribute("FriendlyName", doc->allocate_string(mesh->friendlyName.c_str())));
			meshNode->append_attribute(doc->allocate_attribute("MeshName", doc->allocate_string(mesh->meshName.c_str())));
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

			std::string scaleString = std::to_string(glm::length(meshTransform->scale));
			scaleNode->value(doc->allocate_string(scaleString.c_str()));

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

			if (mesh->textures[0].name != "" || mesh->textures[1].name != "" || mesh->textures[2].name != "" || mesh->textures[3].name != "")
			{
				xml_node<>* texturesNode = doc->allocate_node(rapidxml::node_element, "Textures");

				for (Texture t : mesh->textures)
				{
					xml_node<>* textureNode = doc->allocate_node(rapidxml::node_element, "Texture");
					textureNode->value(doc->allocate_string(t.name.c_str()));

					std::string ratioString = std::to_string(t.ratio);
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

			if (mesh->bUseHeightMap)
			{
				xml_node<>* heightMapNode = doc->allocate_node(rapidxml::node_element, "HeightMap");
				heightMapNode->value(doc->allocate_string(mesh->heightMapName.c_str()));

				miscNode->append_node(heightMapNode);
			}

			if (mesh->bUseSpecular)
			{
				xml_node<>* specularNode = doc->allocate_node(rapidxml::node_element, "UseSpecular");

				miscNode->append_node(specularNode);
			}

			if (mesh->emmisionPower != 1.0f) 
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
	}

	sceneNode->append_node(descNode);
	doc->append_node(sceneNode);

	std::ofstream sceneFile("assets/scenes/" + sceneName + ".xml");
	if (!sceneFile.is_open())
	{
		std::cout << "ERROR could not open sceneFile file from scene saver" << std::endl;
	}

	sceneFile << *doc;

	sceneFile.close();

	doc->clear();
	delete doc;

	return true;
}