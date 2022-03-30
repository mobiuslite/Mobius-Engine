// Vertex shader
#version 420

//uniform mat4 MVP;
uniform mat4 matModel;
uniform mat4 matView;
uniform mat4 matProjection;
uniform mat4 matModelInverseTranspose;// For normal calculation

uniform bool bUseHeightMap;
uniform bool bUseInstancedRendering;
uniform sampler2D heightMap;

uniform vec4 offsets[10];

//uniform bool bUseVertexColour;		// Will default to GL_FALSE, which is zero (0)
//uniform vec3 vertexColourOverride;

in vec4 vColour;
in vec4 vPosition;
in vec4 vNormal;				// Vertex normal X,Y,Z (W ignored)
in vec4 vUVx2;					// 2 x Texture coords (vec4) UV0, UV1
in vec4 vTangent;				// For bump mapping X,Y,Z (W ignored)
in vec4 vBiNormal;				// For bump mapping X,Y,Z (W ignored)


out vec4 fVertexColour;			// used to be "out vec3 color"
out vec4 fVertWorldLocation;
out vec4 fNormal;
out vec4 fUVx2;
out vec4 fVertPosition;
out mat3 TBN;

void main()
{
	// Order of these is important
	//mvp = p * v * matModel; from C++ code
	
	mat4 MVP = matProjection * matView * matModel;
			
	vec4 vertPosition = vPosition;

	if (bUseHeightMap)
	{
		float heightSample = texture(heightMap, vUVx2.xy ).r;		
		vertPosition.y += heightSample;		
	}

	vec4 pos = MVP * vertPosition;
	if (bUseInstancedRendering) 
	{
		pos += offsets[gl_InstanceID];
	}

	gl_Position = pos;		// Used to be: vec4(vPosition, 1.0f);	// Used to be vPos
	
	// The location of the vertex in "world" space (not screen space)
	fVertWorldLocation = matModel * vertPosition;
	fVertPosition = vertPosition;
	// Copy the vertex colour to the fragment shader
	// (if you want the colours from the original file used)
    fVertexColour = vColour;		// Used to be vCol
	
	// Calculate the normal based on any rotation we've applied.
	// This inverse transpose removes scaling and tranlation (movement) 
	// 	from the matrix.
	fNormal = matModelInverseTranspose * normalize(vNormal);
	fNormal = normalize(fNormal);
	
	// Copy the rest of the vertex values:
	fUVx2 = vUVx2;

	mat3 mVector = transpose(inverse(mat3(matModel)));

	vec3 T = normalize(mVector * vTangent.xyz);
	vec3 B = normalize(mVector * vBiNormal.xyz);
	vec3 N = normalize(mVector * vNormal.xyz);

	TBN = mat3(T, B, N);
};
