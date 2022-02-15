#ifndef _cVAOManager_HG_
#define _cVAOManager_HG_

// Will load the models and place them 
// into the vertex and index buffers to be drawn

#include <string>
#include <map>

// The vertex structure 
//	that's ON THE GPU (eventually) 
// So dictated from THE SHADER
//struct sVertex
//{
//	float x, y, z;		
//	float r, g, b;
//};

#include "../sVertex_Types.h"
#include <list>

struct sModelDrawInfo
{
	sModelDrawInfo(); 

	std::string meshName;

	unsigned int VAO_ID;

	unsigned int VertexBufferID;
	unsigned int VertexBuffer_Start_Index;
	unsigned int numberOfVertices;

	unsigned int IndexBufferID;
	unsigned int IndexBuffer_Start_Index;
	unsigned int numberOfIndices;
	unsigned int numberOfTriangles;

	float defaultScale;

	// The "local" (i.e. "CPU side" temporary array)
	sVertex_XYZW_RGBA_N_UV_T_B* pVertices;	//  = 0;
	// The index buffer (CPU side)
	unsigned int* pIndices;
	bool bLoadedFromFile;
	bool bLoadedIntoVAO;
};


class cVAOManager
{
public:

	cVAOManager();
	~cVAOManager() { delete m_pendingLoadingModel; }

	bool LoadModelIntoVAO(std::string fileName, 
						  sModelDrawInfo &drawInfo, 
						  unsigned int shaderProgramID);

	bool LoadPendingModelIntoVAO(std::string fileName,
		sModelDrawInfo& drawInfo);

	void SetShaderProgramID_Threaded(unsigned int shaderId);
	bool LoadModelIntoVAO_Threaded(std::string fileName,
						  sModelDrawInfo& drawInfo);

	// We don't want to return an int, likely
	bool FindDrawInfoByModelName(std::string filename,
								 sModelDrawInfo &drawInfo, bool returnPendingModelIfNotFound = true);

	std::string getLastError(bool bAndClear = true);

private:

	std::list<sModelDrawInfo> listPendingModelsToLoad;
	unsigned int shaderProgramID_ThreadedLoader;

	bool workerThreadIsRunning = false;

	sModelDrawInfo* m_pendingLoadingModel;

	std::map< std::string /*model name*/,
		      sModelDrawInfo /* info needed to draw*/ >
		m_map_ModelName_to_VAOID;

};

#endif	// _cVAOManager_HG_
