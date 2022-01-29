// Fragment shader
#version 420

in vec4 fVertexColour;			// The vertex colour from the original model
in vec4 fVertWorldLocation;
in vec4 fNormal;
in vec4 fUVx2;
in vec4 fVertPosition;
in mat3 TBN;
// Replaces gl_FragColor
out vec4 pixelColour;
out vec4 pixelMatColor;
out vec4 pixelNormal;
out vec4 pixelWorldPos;
out vec4 pixelSpecular;

// The "whole object" colour (diffuse and specular)
uniform vec4 wholeObjectDiffuseColour;	// Whole object diffuse colour
uniform bool bUseWholeObjectDiffuseColour;	// If true, the whole object colour is used (instead of vertex colour)
uniform vec4 wholeObjectSpecularColour;	// Colour of the specular highlight (optional)


// This is used for wireframe or whole object colour. 
// If bUseDebugColour is TRUE, then the fragment colour is "objectDebugColour".
uniform bool bUseDebugColour;	
uniform vec4 objectDebugColour;		

// This will not modulate the colour by the lighting contribution.
// i.e. shows object colour "as is". 
// Used for wireframe or debug type objects
uniform bool bDontLightObject;			// 1 if you want to AVOID lighting
uniform bool bUseSpecular;
// This is the camera eye location (update every frame)
uniform vec4 eyeLocation;

uniform bool bDebugMode;
uniform bool bDebugShowLighting;
uniform bool bDebugShowNormals;

uniform uint passNumber;
const uint RENDER_PASS_0_G_BUFFER_PASS = 0;
const uint RENDER_PASS_1_LIGHT_PASS = 1;
const uint RENDER_PASS_2_EFFECTS_PASS = 2;

struct sLight
{
	vec4 position;			
	vec4 diffuse;	
	vec4 specular;	// rgb = highlight colour, w = power
	vec4 atten;		// x = constant, y = linear, z = quadratic, w = DistanceCutOff
	vec4 direction;	// Spot, directional lights
	vec4 param1;	// x = lightType, y = inner angle, z = outer angle, w = TBD
	                // 0 = pointlight
					// 1 = spot light
					// 2 = directional light
	vec4 param2;	// x = 0 for off, 1 for on
};

const int POINT_LIGHT_TYPE = 0;
const int SPOT_LIGHT_TYPE = 1;
const int DIRECTIONAL_LIGHT_TYPE = 2;


const int NUMBEROFLIGHTS = 15;
uniform sLight theLights[NUMBEROFLIGHTS];  	// 80 uniforms

uniform sampler2D texture_00;
uniform sampler2D texture_01;
uniform sampler2D texture_02;
uniform sampler2D texture_03;

uniform sampler2D texture_MatColor;
uniform sampler2D texture_Normal;
uniform sampler2D texture_WorldPos;
uniform sampler2D texture_Specular;

uniform sampler2D texLightpassColorBuf;

uniform bool bUseAlphaMask;
uniform sampler2D alphaMask;

uniform bool bUseNormalMap;
uniform sampler2D normalMap;
uniform vec2 normalOffset;

uniform bool bUseSkybox;
uniform bool bIsImposter;

uniform vec4 textureRatios;

uniform vec2 screenWidthHeight;

uniform samplerCube skyBox; // GL_TEXTURE_CUBE_MAP

vec4 calcualteLightContrib( vec3 vertexMaterialColour, vec3 vertexNormal, 
                            vec3 vertexWorldPos, vec4 vertexSpecular );

void main()
{
	// This is the pixel colour on the screen.
	// Just ONE pixel, though.
	pixelColour = vec4(0.0f, 0.0f, 0.0f, 1.0f);

	vec4 normals = fNormal;
	vec4 vertexDiffuseColour = fVertexColour;

	if(passNumber == RENDER_PASS_2_EFFECTS_PASS)
	{
		//vec2 UVLookup;
		//UVLookup.x = gl_FragCoord.x / screenWidthHeight.x;
		//UVLookup.y = gl_FragCoord.y / screenWidthHeight.y;
//
		//vec3 sampleColor = texture(texLightpassColorBuf, UVLookup).rgb;
		//pixelColour.rgb = sampleColor;
		//return;
	}

	if(passNumber == RENDER_PASS_1_LIGHT_PASS)
	{
		vec2 UVLookup;
		UVLookup.x = gl_FragCoord.x / screenWidthHeight.x;
		UVLookup.y = gl_FragCoord.y / screenWidthHeight.y;

		vec4 vertDif = texture(texture_MatColor, UVLookup).rgba;
		vec4 vertWorldPos = texture(texture_WorldPos, UVLookup).rgba;

		//If not lit
		//if(vertWorldPos.w == 0.0f)
		//{
			//pixelColour.rgb = vertDif.rgb;
			//pixelColour.a = 1.0f;
			//return;
		//}

		vec4 vertNormal = texture(texture_Normal, UVLookup).rgba;		
		vec4 vertSpecular = texture(texture_Specular, UVLookup).rgba;

		pixelColour = calcualteLightContrib( vertDif.rgb,vertNormal.xyz, vertWorldPos.xyz,vertSpecular.rgba );
											
		pixelColour.a = 1.0f;
		//pixelColour.rg = UVLookup;
		//pixelColour.rgb = vertSpecular.rgb;

		//pixelColour.r = 1.0f;
		return;
	}

	if(passNumber == RENDER_PASS_0_G_BUFFER_PASS)
	{
	
	
	}

	if(bUseSkybox)
	{	
		vec4 skyBoxTexture = texture(skyBox, fVertPosition.xyz);
		//pixelColour = skyBoxTexture;

		pixelMatColor = vec4(skyBoxTexture.rgb, 1.0f);
		pixelWorldPos = vec4(fVertWorldLocation.xyz, 0.0f);

		return;
	}

	if(bUseAlphaMask)
	{
		vec3 alphaMaskTexture = texture(alphaMask, fUVx2.xy).rgb;

		float alphaValue = (alphaMaskTexture.r + alphaMaskTexture.g + alphaMaskTexture.b) / 3.0f;
		//pixelColour.a = alphaValue;

		if(alphaValue < 0.001f)
		{
			discard;
		}
	}

	if(bUseNormalMap)
	{
		vec3 normalMapTexture = texture(normalMap, vec2(fUVx2.x + normalOffset.x, fUVx2.y + normalOffset.y)).rgb;
		normalMapTexture = normalMapTexture * 2.0f - 1.0f;

		normals.xyz = normalize(TBN * normalMapTexture);
	}

	if(bDebugMode && bDebugShowNormals)
	{
		pixelColour.rgb = normals.xyz;
		return;
	}

	//ST = UV
	if(textureRatios.x >  0.0f)
	{
		vec3 textureColour = (texture(texture_00, fUVx2.xy).rgb * textureRatios.x) + (texture(texture_01, fUVx2.xy).rgb * textureRatios.y);

		vertexDiffuseColour = vec4(textureColour, 1.0f);

		if(bIsImposter)
		{
			float averageTexValue = textureColour.r + textureColour.g + textureColour.b;
			averageTexValue /= 3.0f;

			float alphaValue = averageTexValue;

			vertexDiffuseColour.a = alphaValue;
		}
	}
	
	// Use model vertex colours or not?
	if ( bUseWholeObjectDiffuseColour )
	{
		vertexDiffuseColour = wholeObjectDiffuseColour;
	}
	
	// Use debug colour?
	if ( bUseDebugColour )
	{
		// Overwrite the vertex colour with this debug colour
		vertexDiffuseColour = objectDebugColour;	
		pixelMatColor = vec4(objectDebugColour.rgb, 1.0f);
		pixelWorldPos = vec4(fVertWorldLocation.xyz, 0.0f);
	}
	
	// Used for drawing "debug" objects (usually wireframe)
	if ( bDontLightObject )
	{
		pixelMatColor = vertexDiffuseColour;
		//pixelColour = vertexDiffuseColour;
		// Early exit from shader
		return;
	}
	
	vec4 outColour = vec4(vertexDiffuseColour.rgb, 1.0f);

	if((bDebugMode && bDebugShowLighting) || !bDebugMode)
	{
		//outColour = calcualteLightContrib( vertexDiffuseColour.rgb,		
	     //                                   normals.xyz, 		// Normal at the vertex (in world coords)
        //                                    fVertWorldLocation.xyz,	// Vertex WORLD position
		//									wholeObjectSpecularColour.rgba );
	}
	
	//pixelColour = outColour;
	//pixelColour.a = vertexDiffuseColour.a;

	pixelMatColor = vec4(vertexDiffuseColour.rgb, 1.0f);
	pixelNormal = vec4(normalize(normals.xyz), 1.0f);
	pixelWorldPos = vec4(fVertWorldLocation.xyz, 1.0f);

	if(bUseSpecular){
		//pixelSpecular = wholeObjectSpecularColour.rgba;
		pixelSpecular = wholeObjectSpecularColour;
		pixelSpecular.a *= 0.001f;
	}else{
		pixelSpecular = vec4(0.0f, 0.0f, 0.0f, 0.0f);
	}

	
};



// Calculates the colour of the vertex based on the lighting and vertex information:
vec4 calcualteLightContrib( vec3 vertexMaterialColour, vec3 vertexNormal, 
                            vec3 vertexWorldPos, vec4 vertexSpecular )
{
	vec3 norm = normalize(vertexNormal);
	
	vec4 finalObjectColour = vec4( 0.0f, 0.0f, 0.0f, 1.0f );
	
	for ( int index = 0; index < NUMBEROFLIGHTS; index++ )
	{	
		// ********************************************************
		// is light "on"
		if ( theLights[index].param2.x == 0.0f )
		{	// it's off
			continue;
		}
		
		// Cast to an int (note with c'tor)
		int intLightType = int(theLights[index].param1.x);
		
		// We will do the directional light here... 
		// (BEFORE the attenuation, since sunlight has no attenuation, really)
		if ( intLightType == DIRECTIONAL_LIGHT_TYPE )		// = 2
		{
			// This is supposed to simulate sunlight. 
			// SO: 
			// -- There's ONLY direction, no position
			// -- Almost always, there's only 1 of these in a scene
			// Cheapest light to calculate. 

			vec3 lightContrib = theLights[index].diffuse.rgb;
			
			// Get the dot product of the light and normalize
			float dotProduct = dot( -theLights[index].direction.xyz,  
									   normalize(norm.xyz) );	// -1 to 1

			dotProduct = max( 0.0f, dotProduct );		// 0 to 1
		
			lightContrib *= dotProduct;		
			
			finalObjectColour.rgb += (vertexMaterialColour.rgb * theLights[index].diffuse.rgb * lightContrib); 
									 //+ (materialSpecular.rgb * lightSpecularContrib.rgb);
			// NOTE: There isn't any attenuation, like with sunlight.
			// (This is part of the reason directional lights are fast to calculate)


			//return finalObjectColour;		
		}
		
		// Assume it's a point light 
		// intLightType = 0
		
		// Contribution for this light
		vec3 vLightToVertex = theLights[index].position.xyz - vertexWorldPos.xyz;
		float distanceToLight = length(vLightToVertex);	
		vec3 lightVector = normalize(vLightToVertex);
		float dotProduct = dot(lightVector, vertexNormal.xyz);	 
		
		dotProduct = max( 0.0f, dotProduct );	
		
		vec3 lightDiffuseContrib = dotProduct * theLights[index].diffuse.rgb;
			

		// Specular 
		vec3 lightSpecularContrib = vec3(0.0f);
			
		vec3 reflectVector = reflect( -lightVector, norm.xyz);
		//vec3 reflectVector = 2.0 * dot(norm.xyz, lightVector) * norm.xyz - lightVector;

		// Get eye or view vector
		// The location of the vertex in the world to your eye
		vec3 eyeVector = normalize(eyeLocation.xyz - vertexWorldPos.xyz);

		// To simplify, we are NOT using the light specular value, just the object’s.
		float objectSpecularPower = vertexSpecular.a * 1000.0f; 
		//float objectSpecularPower = 10.0f; 
		
//		lightSpecularContrib = pow( max(0.0f, dot( eyeVector, reflectVector) ), objectSpecularPower )
//			                   * vertexSpecular.rgb;	//* theLights[lightIndex].Specular.rgb

		if(dotProduct > 0.0f && vertexSpecular.w > 0.0f)
		{
			lightSpecularContrib = pow( clamp(dot( reflectVector, eyeVector), 0.0f, 1.0f ), objectSpecularPower )
			                   * theLights[index].specular.rgb;
		}
		// Attenuation
		float attenuation = 1.0f / 
				( theLights[index].atten.x + 										
				  theLights[index].atten.y * distanceToLight +						
				  theLights[index].atten.z * distanceToLight*distanceToLight );  	
				  
		// total light contribution is Diffuse + Specular
		lightDiffuseContrib *= attenuation;
		lightSpecularContrib *= attenuation;
		
		
		// But is it a spot light
		if ( intLightType == SPOT_LIGHT_TYPE )		// = 1
		{	
		

			// Yes, it's a spotlight
			// Calcualate light vector (light to vertex, in world)
			vec3 vertexToLight = vertexWorldPos.xyz - theLights[index].position.xyz;

			vertexToLight = normalize(vertexToLight);

			float currentLightRayAngle
					= dot( vertexToLight.xyz, theLights[index].direction.xyz );
					
			currentLightRayAngle = max(0.0f, currentLightRayAngle);

			//vec4 param1;	
			// x = lightType, y = inner angle, z = outer angle, w = TBD

			// Is this inside the cone? 
			float outerConeAngleCos = cos(radians(theLights[index].param1.z));
			float innerConeAngleCos = cos(radians(theLights[index].param1.y));
							
			// Is it completely outside of the spot?
			if ( currentLightRayAngle < outerConeAngleCos )
			{
				// Nope. so it's in the dark
				lightDiffuseContrib = vec3(0.0f, 0.0f, 0.0f);
				lightSpecularContrib = vec3(0.0f, 0.0f, 0.0f);
			}
			else if ( currentLightRayAngle < innerConeAngleCos )
			{
				// Angle is between the inner and outer cone
				// (this is called the penumbra of the spot light, by the way)
				// 
				// This blends the brightness from full brightness, near the inner cone
				//	to black, near the outter cone
				float penumbraRatio = (currentLightRayAngle - outerConeAngleCos) / 
									  (innerConeAngleCos - outerConeAngleCos);
									  
				lightDiffuseContrib *= penumbraRatio;
				lightSpecularContrib *= penumbraRatio;
			}
						
		}// if ( intLightType == 1 )
		
		
					
		finalObjectColour.rgb += (vertexMaterialColour.rgb * lightDiffuseContrib.rgb)
								  + (vertexSpecular.rgb  * lightSpecularContrib.rgb );

	}//for(intindex=0...
	
	finalObjectColour.a = 1.0f;
	
	return finalObjectColour;
}
