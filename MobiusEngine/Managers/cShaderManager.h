#ifndef _cShaderManager_HG_
#define _cShaderManager_HG_

#include <string>
#include <vector>
#include <map>

#include "../GLCommon.h"
#include "../RenderType.h"

//Manages all shaders, allows you to get shaders based on IDs and friendly names
class cShaderManager
{
public:

	class cShader  {
	public:
		cShader();
		~cShader();
		enum eShaderType
		{
			VERTEX_SHADER,
			FRAGMENT_SHADER,
			UNKNOWN
		};
		eShaderType shaderType;
		std::string getShaderTypeString(void);

		unsigned int ID;	// or "name" from OpenGL
		std::vector<std::string> vecSource;
		bool bSourceIsMultiLine;
		std::string fileName;
			
	};

	//Shader data that holds the type of shader it is (normal, shadow, pingpong)
	class cShaderProgram {
	public:
		cShaderProgram(RenderType type = RenderType::Normal) : ID(0), type(type) {};
		~cShaderProgram() {};
		unsigned int ID;	// ID from OpenGL (calls it a "name")
		std::string friendlyName;	// We give it this name

		RenderType type;

		std::map<std::string, GLint> uniformLocations;
	};

	cShaderManager();
	~cShaderManager();

	bool useShaderProgram( unsigned int ID );
	bool useShaderProgram( std::string friendlyName );
	bool createProgramFromFile( std::string friendlyName, 
		                        cShader &vertexShad, 
					            cShader &fragShader, RenderType type);
	void setBasePath( std::string basepath );
	unsigned int getIDFromFriendlyName( std::string friendlyName );

	// Used to load the uniforms. Returns NULL if not found.
	cShaderProgram* pGetShaderProgramFromFriendlyName( std::string friendlyName );


	// Clears last error
	std::string getLastError(void);
private:
	// Returns an empty string if it didn't work
	bool m_loadSourceFromFile( cShader &shader );
	std::string m_basepath;

	bool m_compileShaderFromSource( cShader &shader, std::string &error );
	// returns false if no error
	bool m_wasThereACompileError( unsigned int shaderID, std::string &errorText );
	bool m_wasThereALinkError( unsigned int progID, std::string &errorText );

	std::string m_lastError;

	std::map< unsigned int /*ID*/, cShaderProgram > m_ID_to_Shader;
	std::map< std::string /*name*/, unsigned int /*ID*/ > m_name_to_ID;
};

#endif