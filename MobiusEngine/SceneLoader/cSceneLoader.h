#pragma once

#include <vector>
#include <string>
#include "../Managers/cVAOManager.h"
#include <glad/glad.h>

#include "sModel.h"

#include "../Managers/cBasicTextureManager/cBasicTextureManager.h"

#include <glm/vec3.hpp>
#include "../Managers/cEntityManager.h"

//Loads the scene xml files and loads models into VAO
class cSceneLoader
{
public:
	~cSceneLoader();

	//Gets the instance of the scene loader
	static cSceneLoader* GetSceneLoaderInstance();

	//Loads and parses a json file what contains info about a scene.
	bool LoadScene(std::string sceneName, cBasicTextureManager* textureManager, cEntityManager* entityManager);

	//Loads the scene into the VAO manager
	bool LoadIntoVAO(cVAOManager* vao, GLuint program);

	//Returns the meshes generated from the scene.
	std::vector<sModel>* GetModels();

	glm::vec3 GetCameraStartingPosition();

	bool SaveScene(std::string sceneName, glm::vec3 cameraPos, cEntityManager* entityManager);

private:
	cSceneLoader();

	std::vector<sModel> models;
	glm::vec3 cameraPosition;

	static cSceneLoader _instance;

	bool loadedModels;
};
