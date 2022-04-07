// Vertex shader
#version 420

//uniform mat4 MVP;
uniform mat4 matModel;
uniform mat4 matView;
uniform mat4 matProjection;
uniform mat4 matModelInverseTranspose;// For normal calculation

uniform bool bUseHeightMap;
uniform bool bUseInstancedRendering;

uniform bool bUseWind;
uniform float windTime;
uniform float windSize;

uniform sampler2D heightMap;
uniform sampler2D windMap;

uniform vec3 windDirection;
uniform float windStrength;
uniform mat4 lightSpaceMatrix;

//uniform bool bUseVertexColour;		// Will default to GL_FALSE, which is zero (0)
//uniform vec3 vertexColourOverride;

layout(location = 0) in vec4 vColour;
layout(location = 1) in vec4 vPosition;
layout(location = 2) in vec4 vNormal;				// Vertex normal X,Y,Z (W ignored)
layout(location = 3) in vec4 vUVx2;					// 2 x Texture coords (vec4) UV0, UV1
layout(location = 4) in vec4 vTangent;				// For bump mapping X,Y,Z (W ignored)
layout(location = 5) in vec4 vBiNormal;				// For bump mapping X,Y,Z (W ignored)
layout(location = 6) in vec4 vInstancedOffset;


out vec4 fVertexColour;			// used to be "out vec3 color"
out vec4 fVertWorldLocation;
out vec4 fNormal;
out vec4 fUVx2;
out vec4 fVertPosition;
out mat3 TBN;
out vec4 fLightSpacePos;

void main()
{
	// Order of these is important
	//mvp = p * v * matModel; from C++ code
	
	mat4 MVP = matProjection * matView * matModel;

	vec3 modelPos = vec3(matModel[3][0], matModel[3][1], matModel[3][2]);
			
	vec4 vertPosition = vPosition;

	if (bUseHeightMap)
	{
		float heightSample = texture(heightMap, vUVx2.xy).r;
		vertPosition.y += heightSample;
	}

	if (bUseInstancedRendering)
	{
		vertPosition += vInstancedOffset;
	}

	fVertWorldLocation = matModel * vertPosition;
	fVertWorldLocation.w = 1.0f;

	
	if (bUseWind)
	{
		float windSample = texture(windMap, (modelPos.xz + vInstancedOffset.xz + vec2(windTime)) * windSize).r;
		//Makes 0-1 range into -1 - 1 range
		windSample = windSample * 2.0f - 1.0f;
		windSample *= vUVx2.y * windStrength;

		vertPosition += vec4(normalize(windDirection) * windSample, 1.0f);
	}

	vertPosition.w = 1.0f;
	vec4 pos = MVP * vertPosition;
	
	gl_Position = pos;		// Used to be: vec4(vPosition, 1.0f);	// Used to be vPos
	
	// The location of the vertex in "world" space (not screen space)
	

	fLightSpacePos = lightSpaceMatrix * fVertWorldLocation;

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
