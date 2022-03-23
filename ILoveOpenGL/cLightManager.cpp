#include "cLightManager.h"

#include "GLCommon.h"

cLightManager::cLightManager()
{
	// Set all the values of the lights, etc.



}

sLight::sLight()
{
	// Clear all the values to the unknown ones
	this->position = glm::vec4(0.0f, 0.0f, 0.0f, 0.0f);
	// White light
	this->diffuse = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
	// White specular highlight
	this->specular = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);

	// x = constant, y = linear, z = quadratic, w = DistanceCutOff
	this->atten = glm::vec4(0.0f, 0.1f, 0.01f, 100000.0f);

	this->direction = glm::vec4(0.0f, -1.0f, 0.0f, 0.0f);

	// x = lightType, y = inner angle, z = outer angle, w = TBD
	// 0 = pointlight
	// 1 = spot light
	// 2 = directional light
	this->param1.x = 0;	// Spot light
	this->param1.y = 0.0f;
	this->param1.z = 0.0f;
	this->param1.w = 1.0f;	// not used, so set to 1.0f

	// x = 0 for off, 1 for on		
	this->on = false;
	// Here's the uniform locations of the light values in the shader
	// Settting these to -1 which is the "not found" or unknown uniform location
	this->position_uniform_location = -1;
	this->diffuse_uniform_location = -1;
	this->specular_uniform_location = -1;
	this->atten_uniform_location = -1;
	this->direction_uniform_location = -1;
	this->param1_uniform_location = -1;
	this->param2_uniform_location = -1;
}



void cLightManager::TurnOnLight(unsigned int lightNumber)
{
	if (lightNumber < cLightManager::NUMBER_OF_LIGHTS)
	{
		this->theLights[lightNumber].on = true;
	}
	return;
}

void cLightManager::TurnOffLight(unsigned int lightNumber)
{
	if (lightNumber < cLightManager::NUMBER_OF_LIGHTS)
	{
		this->theLights[lightNumber].on = false;
	}
	return;
}


// This sets up the initial uniform locations from the shader
void cLightManager::SetUpUniformLocations(unsigned int shaderProgram, unsigned int lightIndex)
{
//	struct sLight
//	{
//		vec4 position;
//		vec4 diffuse;
//		vec4 specular;	// rgb = highlight colour, w = power
//		vec4 atten;		// x = constant, y = linear, z = quadratic, w = DistanceCutOff
//		vec4 direction;	// Spot, directional lights
//		vec4 param1;	// x = lightType, y = inner angle, z = outer angle, w = TBD
//						// 0 = pointlight
//						// 1 = spot light
//						// 2 = directional light
//		vec4 param2;	// x = 0 for off, 1 for on
//	};
//	uniform sLight theLights[NUMBEROFLIGHTS];  	// 80 uniforms

	std::string lightString("theLights[" + std::to_string(lightIndex) + "].");

	std::string position = lightString + "position";
	this->theLights[lightIndex].position_uniform_location =
		glGetUniformLocation(shaderProgram, position.c_str());

	std::string diffuse = lightString + "diffuse";
	this->theLights[lightIndex].diffuse_uniform_location =
		glGetUniformLocation(shaderProgram, diffuse.c_str());

	std::string specular = lightString + "specular";
	this->theLights[lightIndex].specular_uniform_location =
		glGetUniformLocation(shaderProgram, specular.c_str());

	std::string atten = lightString + "atten";
	this->theLights[lightIndex].atten_uniform_location =
		glGetUniformLocation(shaderProgram, atten.c_str());

	std::string direction = lightString + "direction";
	this->theLights[lightIndex].direction_uniform_location =
		glGetUniformLocation(shaderProgram, direction.c_str());

	std::string param1 = lightString + "param1";
	this->theLights[lightIndex].param1_uniform_location =
		glGetUniformLocation(shaderProgram, param1.c_str());

	std::string param2 = lightString + "param2";
	this->theLights[lightIndex].param2_uniform_location =
		glGetUniformLocation(shaderProgram, param2.c_str());

	return;
}

// Copies the values from the array into the shader
void cLightManager::CopyLightInfoToShader(void)
{
	for (int i = 0; i < this->NUMBER_OF_LIGHTS; i++)
	{
		sLight curLight = this->theLights[i];

		glUniform4f(this->theLights[i].position_uniform_location,
			curLight.position.x,
			curLight.position.y,
			curLight.position.z,
			curLight.position.w);

		glUniform4f(this->theLights[i].diffuse_uniform_location,
			curLight.diffuse.x * curLight.power,
			curLight.diffuse.y * curLight.power,
			curLight.diffuse.z * curLight.power,
			curLight.diffuse.w * curLight.power);

		glUniform4f(this->theLights[i].specular_uniform_location,
			curLight.specular.x,
			curLight.specular.y,
			curLight.specular.z,
			curLight.specular.w);

		glUniform4f(this->theLights[i].atten_uniform_location,
			curLight.atten.x,
			curLight.atten.y,
			curLight.atten.z,
			curLight.atten.w);

		glUniform4f(this->theLights[i].direction_uniform_location,
			curLight.direction.x,
			curLight.direction.y,
			curLight.direction.z,
			curLight.direction.w);

		glUniform4f(this->theLights[i].param1_uniform_location,
			curLight.param1.x,
			curLight.param1.y,
			curLight.param1.z,
			curLight.param1.w);

		glm::vec4 secondParams = glm::vec4(0.0f);
		secondParams.x = (this->theLights[i].on) ? 1.0f : 0.0f;

		glUniform4f(this->theLights[i].param2_uniform_location,
			secondParams.x,
			secondParams.y,
			secondParams.z,
			secondParams.w);
	}

	return;
}
