#include "../GLCommon.h"

#include "cVAOManager.h"

#include <glm/vec3.hpp>
#include <glm/vec2.hpp>
#include <glm/vec4.hpp>

#include <vector>
#include <fstream>
#include <glm/geometric.hpp>
#include <iostream>


bool LoadPLYModelFromFile(std::string fileName, sModelDrawInfo& drawInfo);

sModelDrawInfo::sModelDrawInfo()
{

	this->VAO_ID = 0;

	this->VertexBufferID = 0;
	this->VertexBuffer_Start_Index = 0;
	this->numberOfVertices = 0;

	this->IndexBufferID = 0;
	this->IndexBuffer_Start_Index = 0;
	this->numberOfIndices = 0;
	this->numberOfTriangles = 0;

	// The "local" (i.e. "CPU side" temporary array)
	this->pVertices = 0;	// or NULL
	this->pIndices = 0;		// or NULL

	// You could store the max and min values of the 
	//  vertices here (determined when you load them):
	//glm::vec3 maxValues;
	//glm::vec3 minValues;

//	scale = 5.0/maxExtent;		-> 5x5x5
	//float maxExtent;

    this->bLoadedFromFile = false;
    this->bLoadedIntoVAO = false;

	return;
}

cVAOManager::cVAOManager()
{
    m_pendingLoadingModel = nullptr;
    this->shaderProgramID_ThreadedLoader = 0;
}

cVAOManager::~cVAOManager()
{
    delete m_pendingLoadingModel;
}

std::vector<sModelDrawInfo> cVAOManager::GetLoadedModels()
{
    std::vector<sModelDrawInfo> returnVector;

    for (std::map<std::string, sModelDrawInfo>::iterator it = this->m_map_ModelName_to_VAOID.begin(); it != this->m_map_ModelName_to_VAOID.end(); it++)
    {
        returnVector.push_back((*it).second);
    }
        
    return returnVector;
}

bool cVAOManager::LoadPendingModelIntoVAO(std::string fileName,
    sModelDrawInfo& drawInfo)
{
    if (this->LoadModelIntoVAO(fileName, drawInfo, this->shaderProgramID_ThreadedLoader))
    {
        this->m_pendingLoadingModel = new sModelDrawInfo();
        *this->m_pendingLoadingModel = drawInfo;

        return true;
    }

    return false;
}

bool cVAOManager::LoadModelIntoVAO_Threaded(std::string fileName,
    sModelDrawInfo& drawInfo)
{
    return false;
}

void cVAOManager::SetShaderProgramID_Threaded(unsigned int shaderProgram)
{
    this->shaderProgramID_ThreadedLoader = shaderProgram;
}

bool cVAOManager::LoadModelIntoVAO(
		std::string fileName, 
		sModelDrawInfo &drawInfo,
	    unsigned int shaderProgramID)

{
	// Load the model from file
	// (We do this here, since if we can't load it, there's 
	//	no point in doing anything else, right?)

	drawInfo.meshName = fileName;

	// TODO: Load the model from file

    std::string fileExtension = fileName.substr(fileName.size() - 3, 3);

    if (fileExtension == "fbx" || fileExtension == "FBX")
    {
        if (!this->LoadFBXModelFromFile(fileName, drawInfo))
        {
            return false;
        }
    }
    else
    {
        if (!LoadPLYModelFromFile(fileName, drawInfo))
        {
            return false;
        }
    }
	// 
	// Model is loaded and the vertices and indices are in the drawInfo struct
	// 

	// Create a VAO (Vertex Array Object), which will 
	//	keep track of all the 'state' needed to draw 
	//	from this buffer...

	// Ask OpenGL for a new buffer ID...
	glGenVertexArrays( 1, &(drawInfo.VAO_ID) );
	// "Bind" this buffer:
	// - aka "make this the 'current' VAO buffer
	glBindVertexArray(drawInfo.VAO_ID);

	// Now ANY state that is related to vertex or index buffer
	//	and vertex attribute layout, is stored in the 'state' 
	//	of the VAO... 


	// NOTE: OpenGL error checks have been omitted for brevity
//	glGenBuffers(1, &vertex_buffer);
	glGenBuffers(1, &(drawInfo.VertexBufferID) );

//	glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, drawInfo.VertexBufferID);
	// sVert vertices[3]
	glBufferData( GL_ARRAY_BUFFER, 
				  sizeof(sVertex_XYZW_RGBA_N_UV_T_B) * drawInfo.numberOfVertices,	// ::g_NumberOfVertsToDraw,	// sizeof(vertices), 
				  (GLvoid*) drawInfo.pVertices,							// pVertices,			//vertices, 
				  GL_STATIC_DRAW );


	// Copy the index buffer into the video card, too
	// Create an index buffer.
	glGenBuffers( 1, &(drawInfo.IndexBufferID) );

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, drawInfo.IndexBufferID);

	glBufferData( GL_ELEMENT_ARRAY_BUFFER,			// Type: Index element array
	              sizeof( unsigned int ) * drawInfo.numberOfIndices, 
	              (GLvoid*) drawInfo.pIndices,
                  GL_STATIC_DRAW );

	// Set the vertex attributes.

	GLint vpos_location = glGetAttribLocation(shaderProgramID, "vPosition");	// program
	glEnableVertexAttribArray(vpos_location);	    // vPos
	glVertexAttribPointer( vpos_location, 4,		// vPos
						   GL_FLOAT, GL_FALSE,
						   sizeof(sVertex_XYZW_RGBA_N_UV_T_B),
						   ( void* ) offsetof(sVertex_XYZW_RGBA_N_UV_T_B, x));
						   //( void* )0);

    GLint vcol_location = glGetAttribLocation(shaderProgramID, "vColour");
	glEnableVertexAttribArray(vcol_location);	    // vCol
	glVertexAttribPointer( vcol_location, 4,		// vCol
						   GL_FLOAT, GL_FALSE,
                            sizeof(sVertex_XYZW_RGBA_N_UV_T_B),
						   ( void* ) offsetof(sVertex_XYZW_RGBA_N_UV_T_B, r));

    GLint vnorm_location = glGetAttribLocation(shaderProgramID, "vNormal");
    glEnableVertexAttribArray(vnorm_location);	    // vCol
    glVertexAttribPointer(vnorm_location, 4,		// vCol
        GL_FLOAT, GL_FALSE,
        sizeof(sVertex_XYZW_RGBA_N_UV_T_B),
        (void*)offsetof(sVertex_XYZW_RGBA_N_UV_T_B, nx));

    GLint vUV_location = glGetAttribLocation(shaderProgramID, "vUVx2");
    glEnableVertexAttribArray(vUV_location);	    // vCol
    glVertexAttribPointer(vUV_location, 4,		// vCol
        GL_FLOAT, GL_FALSE,
        sizeof(sVertex_XYZW_RGBA_N_UV_T_B),
        (void*)offsetof(sVertex_XYZW_RGBA_N_UV_T_B, u0));

    GLint vtang_location = glGetAttribLocation(shaderProgramID, "vTangent");
    glEnableVertexAttribArray(vtang_location);	    // vCol
    glVertexAttribPointer(vtang_location, 4,		// vCol
        GL_FLOAT, GL_FALSE,
        sizeof(sVertex_XYZW_RGBA_N_UV_T_B),
        (void*)offsetof(sVertex_XYZW_RGBA_N_UV_T_B, tx));

    GLint vBiNorm_location = glGetAttribLocation(shaderProgramID, "vBiNormal");
    glEnableVertexAttribArray(vBiNorm_location);	    // vCol
    glVertexAttribPointer(vBiNorm_location, 4,		// vCol
        GL_FLOAT, GL_FALSE,
        sizeof(sVertex_XYZW_RGBA_N_UV_T_B),
        (void*)offsetof(sVertex_XYZW_RGBA_N_UV_T_B, bx));

	// Now that all the parts are set up, set the VAO to zero
	glBindVertexArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	glDisableVertexAttribArray(vpos_location);
	glDisableVertexAttribArray(vcol_location);
	glDisableVertexAttribArray(vnorm_location);
	glDisableVertexAttribArray(vUV_location);
	glDisableVertexAttribArray(vtang_location);
	glDisableVertexAttribArray(vBiNorm_location);

	// Store the draw information into the map
	this->m_map_ModelName_to_VAOID[ drawInfo.meshName ] = drawInfo;

    delete drawInfo.pVertices;
    delete drawInfo.pIndices;

	return true;
}


// We don't want to return an int, likely
bool cVAOManager::FindDrawInfoByModelName(
		std::string filename,
		sModelDrawInfo &drawInfo,
        bool returnPendingModelIfNotFound)
{
	std::map< std::string /*model name*/,
			sModelDrawInfo /* info needed to draw*/ >::iterator 
		itDrawInfo = this->m_map_ModelName_to_VAOID.find( filename );

	// Find it? 
	if ( itDrawInfo == this->m_map_ModelName_to_VAOID.end() )
	{
		// Nope didn't find model

        if (returnPendingModelIfNotFound && this->m_pendingLoadingModel != nullptr)
        {
            drawInfo = *this->m_pendingLoadingModel;
            return true;       
        }
		return false;
	}

	// Else we found the thing to draw
	// ...so 'return' that information
	drawInfo = itDrawInfo->second;
	return true;
}

bool cVAOManager::LoadFBXModelFromFile(std::string fileName, sModelDrawInfo& drawInfo)
{
    const aiScene* scene = assimpImporter.ReadFile("assets/models/" + fileName,
        aiProcess_Triangulate |
        aiProcess_GenSmoothNormals |
        aiProcess_PopulateArmatureData |
        aiProcess_FixInfacingNormals |
        aiProcess_LimitBoneWeights);

    if (scene == nullptr)
    {
        printf("MeshManager::LoadMeshWithAssimp: ERROR: Failed to load file %s\n", fileName.c_str());
        return false;
    }

    if (!scene->HasMeshes())
    {
        printf("MeshManager::LoadMeshWithAssimp: ERROR: Model file does not contain any meshes %s\n", fileName.c_str());
        return false;
    }

    aiMesh* mesh = scene->mMeshes[0];

    bool useRGBA = false;
    bool useNormals = false;
    bool useUV = false;
    bool useBones = false;

    // These structures match the PLY file format
    struct sVertex
    {
        float x, y, z;
        float r, g, b, a;
        float nx, ny, nz;
        float u, v;
    };
    struct sTriangle
    {
        unsigned int vertIndex[3];
    };


    // Read the number of vertices
    drawInfo.numberOfVertices = mesh->mNumVertices;
    drawInfo.numberOfTriangles = mesh->mNumFaces;
    drawInfo.numberOfIndices = mesh->mNumFaces * 3;

    float biggestVertex = 0.0;

    //    sVertex myVertexArray[500];
    std::vector<sVertex> vecVertexArray;    // aka "smart array"
    std::vector<sTriangle> vecTriagleArray;

    std::map<int, int> indiciesTest;

    int indexOfVertex = 0;

    // Now we can read the vertices (in a for loop)
    for (unsigned int faceIndx = 0; faceIndx < drawInfo.numberOfTriangles; faceIndx++)
    {
        aiFace curFace = mesh->mFaces[faceIndx];
        sTriangle tempTri;

        for (unsigned int idx = 0; idx != 3; idx++)
        {
            sVertex tempVertex;
            unsigned int index = curFace.mIndices[idx];

            //Vertex doesn't exist, create a new one
            std::map<int, int>::iterator indiciesIt = indiciesTest.find(index);
            if (indiciesIt == indiciesTest.end())
            {
                aiVector3D pos = mesh->mVertices[index];
                tempVertex.x = pos.x;
                tempVertex.y = pos.y;
                tempVertex.z = pos.z;

                if (mesh->HasTextureCoords(0))
                {
                    aiVector3D uv = mesh->mTextureCoords[0][index];
                    tempVertex.u = uv.x;
                    tempVertex.v = uv.y;
                    useUV = true;
                }

                if (mesh->HasNormals())
                {
                    aiVector3D normals = mesh->mNormals[index];
                    tempVertex.nx = normals.x;
                    tempVertex.ny = normals.y;
                    tempVertex.nz = normals.z;
                    useNormals = true;
                }

                if (mesh->HasVertexColors(0))
                {
                    aiColor4D* test = mesh->mColors[index];
                    tempVertex.r = test->r;
                    tempVertex.g = test->g;
                    tempVertex.b = test->b;
                    tempVertex.a = test->a;
                    useRGBA = true;

                }

                vecVertexArray.push_back(tempVertex);

                tempTri.vertIndex[idx] = indexOfVertex;
                indiciesTest.insert(std::pair<int, int>(index, indexOfVertex));

                indexOfVertex++;
            }
            //Vertex was already created, don't create a new one, just get the one that was already created
            else
            {
                std::pair<int, int> indexDup = *indiciesIt;
                tempTri.vertIndex[idx] = indexDup.second;
            }
        }

        vecTriagleArray.push_back(tempTri);
    }
    float thisBiggestVertex = glm::distance(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(mesh->mAABB.mMax.x, mesh->mAABB.mMax.y, mesh->mAABB.mMax.z));

    // Allocate the amount of space we need for the GPU side arrays
    drawInfo.pVertices = new sVertex_XYZW_RGBA_N_UV_T_B[drawInfo.numberOfVertices];
    drawInfo.pIndices = new unsigned int[drawInfo.numberOfIndices];


    // Copy the vertices from the PLY format vector
    //  to the one we'll use to draw in the GPU
    for (unsigned int index = 0; index != drawInfo.numberOfVertices; index++)
    {
        drawInfo.pVertices[index].x = vecVertexArray[index].x;
        drawInfo.pVertices[index].y = vecVertexArray[index].y;
        drawInfo.pVertices[index].z = vecVertexArray[index].z;
        drawInfo.pVertices[index].w = 1.0f;

        if (useNormals)
        {
            drawInfo.pVertices[index].nx = vecVertexArray[index].nx;
            drawInfo.pVertices[index].ny = vecVertexArray[index].ny;
            drawInfo.pVertices[index].nz = vecVertexArray[index].nz;
            drawInfo.pVertices[index].nw = 1.0f;
        }
        else
        {
            drawInfo.pVertices[index].nx = 1.0f;
            drawInfo.pVertices[index].ny = 1.0f;
            drawInfo.pVertices[index].nz = 1.0f;
            drawInfo.pVertices[index].nw = 1.0f;
        }

        if (useRGBA)
        {
            drawInfo.pVertices[index].r = vecVertexArray[index].r;
            drawInfo.pVertices[index].g = vecVertexArray[index].g;
            drawInfo.pVertices[index].b = vecVertexArray[index].b;
            drawInfo.pVertices[index].a = vecVertexArray[index].a;
        }
        else
        {
            drawInfo.pVertices[index].r = 1.0f;
            drawInfo.pVertices[index].g = 1.0f;
            drawInfo.pVertices[index].b = 1.0f;
            drawInfo.pVertices[index].a = 1.0f;
        }

        if (useUV)
        {
            drawInfo.pVertices[index].u0 = vecVertexArray[index].u;
            drawInfo.pVertices[index].v0 = vecVertexArray[index].v;
        }
    }

    // Copy the triangle ("index") values to the index (element) array
    unsigned int elementIndex = 0;
    for (unsigned int triIndex = 0; triIndex < drawInfo.numberOfTriangles;
        triIndex++, elementIndex += 3)
    {
        drawInfo.pIndices[elementIndex + 0] = vecTriagleArray[triIndex].vertIndex[0];
        drawInfo.pIndices[elementIndex + 1] = vecTriagleArray[triIndex].vertIndex[1];
        drawInfo.pIndices[elementIndex + 2] = vecTriagleArray[triIndex].vertIndex[2];

        if (useNormals && useUV)
        {
            sVertex_XYZW_RGBA_N_UV_T_B* vert1 = &drawInfo.pVertices[drawInfo.pIndices[elementIndex + 0]];
            sVertex_XYZW_RGBA_N_UV_T_B* vert2 = &drawInfo.pVertices[drawInfo.pIndices[elementIndex + 1]];
            sVertex_XYZW_RGBA_N_UV_T_B* vert3 = &drawInfo.pVertices[drawInfo.pIndices[elementIndex + 2]];

            glm::vec3 tangent, biTangent;
            glm::vec3 e1 = glm::vec3(vert2->x, vert2->y, vert2->z) - glm::vec3(vert1->x, vert1->y, vert1->z);
            glm::vec3 e2 = glm::vec3(vert3->x, vert3->y, vert3->z) - glm::vec3(vert1->x, vert1->y, vert1->z);
            glm::vec2 dUV1 = glm::vec2(vert2->u0, vert2->v0) - glm::vec2(vert1->u0, vert1->v0);
            glm::vec2 dUV2 = glm::vec2(vert3->u0, vert3->v0) - glm::vec2(vert1->u0, vert1->v0);

            GLfloat f = 1.0f / (dUV1.x * dUV2.y - dUV2.x * dUV1.y);

            tangent.x = f * (dUV2.y * e1.x - dUV1.y * e2.x);
            tangent.y = f * (dUV2.y * e1.y - dUV1.y * e2.y);
            tangent.z = f * (dUV2.y * e1.z - dUV1.y * e2.z);
            tangent = glm::normalize(tangent);

            biTangent.x = f * (-dUV2.x * e1.x - dUV1.x * e2.x);
            biTangent.y = f * (-dUV2.x * e1.y - dUV1.x * e2.y);
            biTangent.z = f * (-dUV2.x * e1.z - dUV1.x * e2.z);
            biTangent = glm::normalize(biTangent);

            vert1->tx = tangent.x;
            vert1->ty = tangent.y;
            vert1->tz = tangent.z;
            vert1->tw = 1.0f;

            vert2->tx = tangent.x;
            vert2->ty = tangent.y;
            vert2->tz = tangent.z;
            vert2->tw = 1.0f;

            vert3->tx = tangent.x;
            vert3->ty = tangent.y;
            vert3->tz = tangent.z;
            vert3->tw = 1.0f;

            vert1->bx = biTangent.x;
            vert1->by = biTangent.y;
            vert1->bz = biTangent.z;
            vert1->bw = 1.0f;

            vert2->bx = biTangent.x;
            vert2->by = biTangent.y;
            vert2->bz = biTangent.z;
            vert2->bw = 1.0f;

            vert3->bx = biTangent.x;
            vert3->by = biTangent.y;
            vert3->bz = biTangent.z;
            vert3->bw = 1.0f;
        }

    }

    return true;

}

bool LoadPLYModelFromFile(std::string fileName, sModelDrawInfo& drawInfo)
{
    bool useRGBA = false;
    bool useNormals = false;
    bool useUV = false;
    bool textureNumber = false;

    // These structures match the PLY file format
    struct sVertex
    {
        float x, y, z;
        float r, g, b, a;
        float nx, ny, nz;
        float u, v;
    };
    struct sTriangle
    {
        unsigned int vertIndex[3];
    };

    std::ifstream theFile("assets/models/" + fileName);

    //    std::ifstream theFile("GalacticaOriginal_ASCII_no_text.ply");
        //    std::ifstream theFile( fileName.c_str() );

            // Did it open?
    if (!theFile.is_open())          // theFile.is_open() == false
    {
        return false;
    }

    // The file is good to go.

    std::string nextToken;

    // Scan until we find the word "vertex"...
    while (theFile >> nextToken)
    {
         if (nextToken == "vertex")
         {
            break;  // Exits the while loop
         }
    }

    // Read the number of vertices
    theFile >> drawInfo.numberOfVertices;


    while (theFile >> nextToken)
    {
        if (nextToken == "red")
            useRGBA = true;
        else if (nextToken == "nx")
            useNormals = true;
        else if (nextToken == "texture_u")
            useUV = true;
        else if (nextToken == "face")
            break;   
    }

    // Read the number of triangles (aka "faces")
    theFile >> drawInfo.numberOfTriangles;

    // Scan until we find the word "end_header"...
    while (theFile >> nextToken)
    {
        if (nextToken == "texnumber")
        {
            textureNumber = true;
        }
        if (nextToken == "end_header")
        {
            break;  // Exits the while loop
        }
    }

    float biggestVertex = 0.0;

    //    sVertex myVertexArray[500];
    std::vector<sVertex> vecVertexArray;    // aka "smart array"

    // Now we can read the vertices (in a for loop)
    for (unsigned int index = 0; index < drawInfo.numberOfVertices; index++)
    {
        float thisBiggestVertex = 0.0;

        sVertex tempVertex;

        theFile >> tempVertex.x;
        theFile >> tempVertex.y;
        theFile >> tempVertex.z;

        //Gets the biggest vertex (Used for default scaling so everything is normalized when adding new meshes in program.)
        thisBiggestVertex = std::max(std::abs(tempVertex.x), std::abs(tempVertex.y));
        thisBiggestVertex = std::max(std::abs(tempVertex.z), biggestVertex);
        if (thisBiggestVertex > biggestVertex)
            biggestVertex = thisBiggestVertex;

        if (useNormals)
        {
            theFile >> tempVertex.nx;
            theFile >> tempVertex.ny;
            theFile >> tempVertex.nz;
        }

        if (useRGBA)
        {
            theFile >> tempVertex.r;
            tempVertex.r /= 255.0f;

            theFile >> tempVertex.g;
            tempVertex.g /= 255.0f;

            theFile >> tempVertex.b;
            tempVertex.b /= 255.0f;

            theFile >> tempVertex.a;
            tempVertex.a /= 255.0f;
        }

        if (useUV)
        {
            theFile >> tempVertex.u;
            theFile >> tempVertex.v;        
        }

        //        theFile >> tempVertex.c;
        //       theFile >> tempVertex.i;

                //vecVertexArray[index] = tempVertex;
                // "Add to the end of the vector"
                // "push" == "add", "back" == "end"
        vecVertexArray.push_back(tempVertex);       // Add thing at end of smart array

    }

    std::vector<sTriangle> vecTriagleArray;    // aka "smart array"

    // Now we can read the triangles (in a for loop)
    for (unsigned int index = 0; index < drawInfo.numberOfTriangles; index++)
    {
        sTriangle tempTri;
        int discardThis;

        // 3 3087 3639 5315 
        theFile >> discardThis; // The "3" at the start
        theFile >> tempTri.vertIndex[0];
        theFile >> tempTri.vertIndex[1];
        theFile >> tempTri.vertIndex[2];

        if (textureNumber)
        {
            int discard;
            theFile >> discard;
        }

        vecTriagleArray.push_back(tempTri);
    }



    // Allocate the amount of space we need for the GPU side arrays
    drawInfo.pVertices = new sVertex_XYZW_RGBA_N_UV_T_B[drawInfo.numberOfVertices];

    // There are 3 indices per triangle...
    drawInfo.numberOfIndices = drawInfo.numberOfTriangles * 3;
    drawInfo.pIndices = new unsigned int[drawInfo.numberOfIndices];


    // Copy the vertices from the PLY format vector
    //  to the one we'll use to draw in the GPU
    for (unsigned int index = 0; index != drawInfo.numberOfVertices; index++)
    {
        drawInfo.pVertices[index].x = vecVertexArray[index].x;
        drawInfo.pVertices[index].y = vecVertexArray[index].y;
        drawInfo.pVertices[index].z = vecVertexArray[index].z;
        drawInfo.pVertices[index].w = 1.0f;

        if (useNormals)
        {
            drawInfo.pVertices[index].nx = vecVertexArray[index].nx;
            drawInfo.pVertices[index].ny = vecVertexArray[index].ny;
            drawInfo.pVertices[index].nz = vecVertexArray[index].nz;
            drawInfo.pVertices[index].nw = 1.0f;
        }
        else
        {
            drawInfo.pVertices[index].nx = 1.0f;
            drawInfo.pVertices[index].ny = 1.0f;
            drawInfo.pVertices[index].nz = 1.0f;
            drawInfo.pVertices[index].nw = 1.0f;
        }

        if (useRGBA)
        {
            drawInfo.pVertices[index].r = vecVertexArray[index].r;
            drawInfo.pVertices[index].g = vecVertexArray[index].g;
            drawInfo.pVertices[index].b = vecVertexArray[index].b;
            drawInfo.pVertices[index].a = vecVertexArray[index].a;
        }
        else
        {
            drawInfo.pVertices[index].r = 1.0f;
            drawInfo.pVertices[index].g = 1.0f;
            drawInfo.pVertices[index].b = 1.0f;
            drawInfo.pVertices[index].a = 1.0f;
        }

        if (useUV)
        {
            drawInfo.pVertices[index].u0 = vecVertexArray[index].u;
            drawInfo.pVertices[index].v0 = vecVertexArray[index].v;
        }      
    }

    // Copy the triangle ("index") values to the index (element) array
    unsigned int elementIndex = 0;
    for (unsigned int triIndex = 0; triIndex < drawInfo.numberOfTriangles; 
         triIndex++, elementIndex += 3)
    {
        drawInfo.pIndices[elementIndex + 0] = vecTriagleArray[triIndex].vertIndex[0];
        drawInfo.pIndices[elementIndex + 1] = vecTriagleArray[triIndex].vertIndex[1];
        drawInfo.pIndices[elementIndex + 2] = vecTriagleArray[triIndex].vertIndex[2];

        if (useNormals && useUV)
        {
            sVertex_XYZW_RGBA_N_UV_T_B* vert1 = &drawInfo.pVertices[drawInfo.pIndices[elementIndex + 0]];
            sVertex_XYZW_RGBA_N_UV_T_B* vert2 = &drawInfo.pVertices[drawInfo.pIndices[elementIndex + 1]];
            sVertex_XYZW_RGBA_N_UV_T_B* vert3 = &drawInfo.pVertices[drawInfo.pIndices[elementIndex + 2]];

            glm::vec3 tangent, biTangent;
            glm::vec3 e1 = glm::vec3(vert2->x, vert2->y, vert2->z) - glm::vec3(vert1->x, vert1->y, vert1->z);
            glm::vec3 e2 = glm::vec3(vert3->x, vert3->y, vert3->z) - glm::vec3(vert1->x, vert1->y, vert1->z);
            glm::vec2 dUV1 = glm::vec2(vert2->u0, vert2->v0) - glm::vec2(vert1->u0, vert1->v0);
            glm::vec2 dUV2 = glm::vec2(vert3->u0, vert3->v0) - glm::vec2(vert1->u0, vert1->v0);

            GLfloat f = 1.0f / (dUV1.x * dUV2.y - dUV2.x * dUV1.y);

            tangent.x = f * (dUV2.y * e1.x - dUV1.y * e2.x);
            tangent.y = f * (dUV2.y * e1.y - dUV1.y * e2.y);
            tangent.z = f * (dUV2.y * e1.z - dUV1.y * e2.z);
            tangent = glm::normalize(tangent);

            biTangent.x = f * (-dUV2.x * e1.x - dUV1.x * e2.x);
            biTangent.y = f * (-dUV2.x * e1.y - dUV1.x * e2.y);
            biTangent.z = f * (-dUV2.x * e1.z - dUV1.x * e2.z);
            biTangent = glm::normalize(biTangent);

            vert1->tx = tangent.x;
            vert1->ty = tangent.y;
            vert1->tz = tangent.z;
            vert1->tw = 1.0f;

            vert2->tx = tangent.x;
            vert2->ty = tangent.y;
            vert2->tz = tangent.z;
            vert2->tw = 1.0f;

            vert3->tx = tangent.x;
            vert3->ty = tangent.y;
            vert3->tz = tangent.z;
            vert3->tw = 1.0f;

            vert1->bx = biTangent.x;
            vert1->by = biTangent.y;
            vert1->bz = biTangent.z;
            vert1->bw = 1.0f;

            vert2->bx = biTangent.x;
            vert2->by = biTangent.y;
            vert2->bz = biTangent.z;
            vert2->bw = 1.0f;

            vert3->bx = biTangent.x;
            vert3->by = biTangent.y;
            vert3->bz = biTangent.z;
            vert3->bw = 1.0f;
        }

    }

    theFile.close();
    return true;
}