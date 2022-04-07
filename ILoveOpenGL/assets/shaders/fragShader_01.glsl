// Fragment shader
#version 420

in vec4 fVertexColour;			// The vertex colour from the original model
in vec4 fVertWorldLocation;
in vec4 fNormal;
in vec4 fUVx2;
in vec4 fVertPosition;
in mat3 TBN;
in vec4 fLightSpacePos;

// Replaces gl_FragColor
layout (location = 0)out vec4 pixelColour;
layout (location = 1)out vec4 pixelMatColor;
layout (location = 2)out vec4 pixelNormal;
layout (location = 3)out vec4 pixelWorldPos;
//layout (location = 4)out vec4 pixelSpecular;
layout (location = 5)out vec4 pixelBrightColour;
layout (location = 6)out vec4 pixelEmmision;
layout (location = 7)out vec4 pixelLightSpacePos;


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

uniform vec3 emmision;
uniform float shadowBias;

uniform float roughness;
uniform float metallic;

uniform float brightness;

//tiling xy, offset zw
uniform vec4 tilingAndOffset;

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


const int NUMBEROFLIGHTS = 10;
uniform sLight theLights[NUMBEROFLIGHTS];  	// 80 uniforms

uniform sampler2D texture_00;
uniform sampler2D texture_01;
uniform sampler2D texture_02;
uniform sampler2D texture_03;

uniform sampler2D texture_MatColor;
uniform sampler2D texture_Normal;
uniform sampler2D texture_WorldPos;
uniform sampler2D texture_LightSpacePos;
//uniform sampler2D texture_Specular;
uniform sampler2D texture_Emmision;

uniform sampler2D texLightpassColorBuf;
uniform sampler2D bloomMapColorBuf;
uniform sampler2D shadowMapColorBuf;

uniform bool bUseAlphaMask;
uniform sampler2D alphaMask;

uniform bool bUseNormalMap;
uniform sampler2D normalMap;
uniform vec2 normalOffset;

uniform bool bUseSkybox;
uniform bool bIsImposter;
uniform bool bShowBloom;

uniform bool bUseSkyboxReflections;

uniform vec4 textureRatios;

uniform vec2 screenWidthHeight;

//x = gamma;
//y = exposure;
//z use exposure based tonemapping > 0.5f
//w = bloom threshhold
uniform vec4 postprocessingVariables;
uniform float ambientPower;

uniform samplerCube skyBox; // GL_TEXTURE_CUBE_MAP

const float PI = 3.14159265359;

float DistributionGGX(vec3 N, vec3 H, float roughness);
float GeometrySchlickGGX(float NdotV, float roughness);
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness);
vec3 fresnelSchlick(float cosTheta, vec3 F0);

vec3 PBR(vec3 albedo, vec3 normal, vec3 worldPos, vec4 lightSpacePos, float roughness, float metallic);

vec4 calcualteLightContrib( vec3 vertexMaterialColour, vec3 vertexNormal, 
                            vec3 vertexWorldPos, vec4 vertexSpecular, vec4 lightSpacePos);

float ShadowCalculation(vec4 fragPosLightSpace, vec3 normal, vec3 lightDir)
{
	// perform perspective divide
	vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
	// transform to [0,1] range
	projCoords = projCoords * 0.5 + 0.5;
	// get closest depth value from light's perspective (using [0,1] range fragPosLight as coords)
	float closestDepth = texture(shadowMapColorBuf, projCoords.xy).r;
	// get depth of current fragment from light's perspective
	float currentDepth = projCoords.z;
	// check whether current frag pos is in shadow

	//float bias = max(0.05 * (1.0 - dot(normal, lightDir)), 0.005);
	float bias = shadowBias;

	float shadow = currentDepth - bias > closestDepth ? 1.0 : 0.0;

	if (currentDepth > 1.0)
		shadow = 0.0;

	return shadow;
}

void main()
{
	// This is the pixel colour on the screen.
	// Just ONE pixel, though.
	//pixelColour = vec4(0.0f, 0.0f, 0.0f, 1.0f);

	vec4 normals = fNormal;
	vec4 vertexDiffuseColour = fVertexColour;
	//pixelFirstPass = vec4(1.0f, 0.0f, 0.0f, 1.0f);
	if(passNumber == RENDER_PASS_2_EFFECTS_PASS)
	{
		vec3 finalColour = vec3(0.0f, 0.0f, 0.0f);

		vec2 UVLookup;
		UVLookup.x = gl_FragCoord.x / screenWidthHeight.x;
		UVLookup.y = gl_FragCoord.y / screenWidthHeight.y;
//		

		finalColour = texture(texLightpassColorBuf, UVLookup).rgb;
		if (bShowBloom)
		{
			vec3 bloomColour = texture(bloomMapColorBuf, UVLookup).rgb;
			finalColour += bloomColour;
		}

		//Tonemapping for HDR	
		vec3 mapped = finalColour;
		if(postprocessingVariables.z > 0.5f)
		{
			//Reinhard tone mapping
			mapped = finalColour / (finalColour + vec3(1.0));
		}
		//postprocessingVariables.z == 0.0f = use exposure.
		else
		{   // exposure tone mapping                //Exposure
			mapped = vec3(1.0) - exp(-finalColour * postprocessingVariables.y);
		}
		// gamma correction 
		mapped = pow(mapped, vec3(1.0/postprocessingVariables.x));
  
		pixelColour.rgb = mapped;
		pixelColour.a = 1.0f;

		return;
	}

	if(passNumber == RENDER_PASS_1_LIGHT_PASS)
	{	
		vec2 UVLookup;
		UVLookup.x = gl_FragCoord.x / screenWidthHeight.x;
		UVLookup.y = gl_FragCoord.y / screenWidthHeight.y;

		//Sends the textures back to their fbo buffers, so we can display them for debugging purposes
		pixelMatColor = texture(texture_MatColor, UVLookup).rgba;

		//w value = roughness
		pixelNormal = texture(texture_Normal, UVLookup).rgba;
		float lightingRoughness = pixelNormal.w;

		pixelWorldPos = texture(texture_WorldPos, UVLookup).rgba;
		pixelLightSpacePos = texture(texture_LightSpacePos, UVLookup).rgba;
		//pixelSpecular = texture(texture_Specular, UVLookup).rgba;

		pixelEmmision = texture(texture_Emmision, UVLookup).rgba;
		float lightingMetallic = pixelEmmision.w;

		float depthValue = texture(shadowMapColorBuf, UVLookup).r;

		//If not lit
		if(pixelWorldPos.w == 0.0f)
		{
			pixelColour.rgb = pixelMatColor.rgb;
		}
		else
		{
			//pixelColour = calcualteLightContrib( pixelMatColor.rgb, pixelNormal.xyz, pixelWorldPos.xyz, pixelSpecular.rgba, pixelLightSpacePos);		
			//pixelColour.a = 1.0f;

			vec3 pbrColor = PBR(pixelMatColor.rgb, pixelNormal.xyz, pixelWorldPos.xyz, pixelLightSpacePos, lightingRoughness, lightingMetallic);
			
			
			vec3 ambient = ambientPower * pixelMatColor.rgb;
			vec3 color = ambient + pbrColor;
			
			pixelColour = vec4(color, 1.0);
		}
		
		pixelColour.rgb += pixelEmmision.rgb;

		//Get bright parts of screen for bloom
		float brightness = dot(pixelColour.rgb, vec3(0.2126f, 0.7152f, 0.0722f));
		if(brightness > postprocessingVariables.w)
		{
			pixelBrightColour = vec4(pixelColour.rgb, 1.0f);
		}
		else
		{
			pixelBrightColour = vec4(0.0f, 0.0f, 0.0f, 1.0f);
		}

		return;
	}

	if(passNumber == RENDER_PASS_0_G_BUFFER_PASS)
	{
	
	
	}

	if(bUseSkybox)
	{	
		vec4 skyBoxTexture = texture(skyBox, fVertPosition.xyz);
		//pixelColour = skyBoxTexture;

		pixelMatColor = pow(vec4(skyBoxTexture.rgb * brightness, 1.0f), vec4(postprocessingVariables.x));	
		pixelMatColor.w = 1.0f;
		pixelWorldPos = vec4(fVertWorldLocation.xyz, 0.0f);

		pixelEmmision = vec4(emmision, metallic);

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
		vec3 textureColour = (texture(texture_00, vec2(fUVx2.x * tilingAndOffset.x, fUVx2.y * tilingAndOffset.y)).rgb * textureRatios.x) + (texture(texture_01, fUVx2.xy).rgb * textureRatios.y);

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
		pixelMatColor = pow(vec4(vertexDiffuseColour.rgb, 1.0f), vec4(postprocessingVariables.x));
		pixelMatColor.w = 0.0f;
		//pixelColour = vertexDiffuseColour;
		// Early exit from shader
		return;
	}

	if(bUseSkyboxReflections)
	{
		vec3 cameraDir = normalize(fVertWorldLocation.xyz - eyeLocation.xyz);
		vec3 reflection = reflect(cameraDir, normalize(normals.xyz));
		vertexDiffuseColour.rgb += texture(skyBox, reflection).rgb * 2f;
	}

	//Inverse gamma, to fix the issue of color correcting twice
	//Most textures are already in linear space, because they are created by an artist wit their eyes
	//and monitors are already corrected
	//So using gamma correction again would correct them twice
	pixelMatColor = pow(vec4(vertexDiffuseColour.rgb * brightness, 1.0f), vec4(postprocessingVariables.x));
	pixelMatColor.w = 1.0f;

	pixelNormal = vec4(normalize(normals.xyz), roughness);

	pixelWorldPos = vec4(fVertWorldLocation.xyz, 1.0f);
	pixelLightSpacePos = fLightSpacePos;

	pixelEmmision = vec4(emmision, metallic);

	//if(bUseSpecular){
	//	//pixelSpecular = wholeObjectSpecularColour.rgba;
	//	pixelSpecular = wholeObjectSpecularColour;
	//}else{
	//	pixelSpecular = vec4(0.0f, 0.0f, 0.0f, 0.0f);
	//}	
};

vec3 PBR(vec3 albedo, vec3 normal, vec3 worldPos, vec4 lightSpacePos, float roughness, float metallic)
{
	vec3 N = normalize(normal);
	vec3 V = normalize(eyeLocation.xyz - worldPos);

	vec3 F0 = vec3(0.04);
	F0 = mix(F0, albedo, metallic);

	vec3 Lo = vec3(0.0);
	for (int index = 0; index < NUMBEROFLIGHTS; index++)
	{
		if (theLights[index].param2.x == 0.0f)
		{	// it's off
			continue;
		}

		vec3 lightVec;
		float attenuation = theLights[index].atten.z;

		int intLightType = int(theLights[index].param1.x);

		if (intLightType == DIRECTIONAL_LIGHT_TYPE)		// = 2
		{
			//lightVec = theLights[index].direction.xyz;
			lightVec = theLights[index].position.xyz;
			attenuation = 1.0;
		}
		else
		{
			lightVec = theLights[index].position.xyz - worldPos;
			float distanceToLight = length(lightVec);

			if (distanceToLight > theLights[index].atten.w)
			{
				continue;
			}
		}

		vec3 L = normalize(lightVec);
		vec3 H = normalize(V + L);		
		vec3 radiance = theLights[index].diffuse.rgb * attenuation;

		float NDF = DistributionGGX(N, H, roughness);
		float G = GeometrySmith(N, V, L, roughness);
		vec3 F = fresnelSchlick(max(dot(V, H), 0.0), F0);

		vec3 kS = F;
		vec3 kD = vec3(1.0) - kS;
		kD *= 1.0 - metallic;

		vec3 numerator = NDF * G * F;
		float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001;
		vec3 specular = numerator / denominator;

		float NdotL = max(dot(N, L), 0.0);

		float shadow = 0.0f;
		if (intLightType == DIRECTIONAL_LIGHT_TYPE)		// = 2
		{
			shadow = ShadowCalculation(lightSpacePos, N, L);
		}

		Lo += ((kD * albedo / PI + specular) * radiance * NdotL) * (1.0f - shadow);
	}
	return Lo;
}

float DistributionGGX(vec3 N, vec3 H, float roughness)
{
	float a = roughness * roughness;
	float a2 = a * a;
	float NdotH = max(dot(N, H), 0.0);
	float NdotH2 = NdotH * NdotH;

	float num = a2;
	float denom = (NdotH2 * (a2 - 1.0) + 1.0);
	denom = PI * denom * denom;

	return num / denom;
}

float GeometrySchlickGGX(float NdotV, float roughness)
{
	float r = (roughness + 1.0);
	float k = (r * r) / 8.0;

	float num = NdotV;
	float denom = NdotV * (1.0 - k) + k;

	return num / denom;
}
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
	float NdotV = max(dot(N, V), 0.0);
	float NdotL = max(dot(N, L), 0.0);
	float ggx2 = GeometrySchlickGGX(NdotV, roughness);
	float ggx1 = GeometrySchlickGGX(NdotL, roughness);

	return ggx1 * ggx2;
}

vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
	return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

// Calculates the colour of the vertex based on the lighting and vertex information:
vec4 calcualteLightContrib( vec3 vertexMaterialColour, vec3 vertexNormal, 
                            vec3 vertexWorldPos, vec4 vertexSpecular, vec4 lightSpacePos)
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
									   norm.xyz );	// -1 to 1

			dotProduct = max( 0.0f, dotProduct );		// 0 to 1
		

			vec3 ambient = ambientPower * lightContrib;

			lightContrib *= dotProduct;				
			vec3 diffuse = theLights[index].diffuse.rgb * lightContrib; 

			vec3 lightVector = normalize(theLights[index].direction.xyz);

			vec3 eyeVector = normalize(eyeLocation.xyz - vertexWorldPos.xyz);
			vec3 halfwayVec = normalize(lightVector + eyeVector);
			float objectSpecularPower = vertexSpecular.a;

			vec3 lightSpecularContrib = vec3(0.0f);
			if (dotProduct > 0.0f && vertexSpecular.w > 0.0f)
			{
				lightSpecularContrib = pow(clamp(dot(norm.xyz, halfwayVec), 0.0f, 1.0f), objectSpecularPower)
					* theLights[index].specular.rgb;
			}

			vec3 specular = vertexSpecular.rgb * lightSpecularContrib.rgb;

			float shadow = ShadowCalculation(lightSpacePos, norm, lightVector);

			//finalObjectColour += vec4((shadow * (diffuse + specular) + ambient) * vertexMaterialColour.rgb, 1.0f);
			finalObjectColour += vec4((ambient + (1.0 - shadow) * (diffuse + specular)) * vertexMaterialColour.rgb, 1.0f);
			//finalObjectColour += vec4(vec3(shadow), 1.0f);

			continue;
		}
		
		// Assume it's a point light 
		// intLightType = 0
		
		// Contribution for this light
		vec3 vLightToVertex = theLights[index].position.xyz - vertexWorldPos.xyz;
		float distanceToLight = length(vLightToVertex);	

		if(distanceToLight > theLights[index].atten.w)
		{
			continue;
		}

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
		vec3 halfwayVec = normalize(lightVector + eyeVector);
		// To simplify, we are NOT using the light specular value, just the object’s.
		float objectSpecularPower = vertexSpecular.a; 
		//float objectSpecularPower = 10.0f; 
		
//		lightSpecularContrib = pow( max(0.0f, dot( eyeVector, reflectVector) ), objectSpecularPower )
//			                   * vertexSpecular.rgb;	//* theLights[lightIndex].Specular.rgb

		if(dotProduct > 0.0f && vertexSpecular.w > 0.0f)
		{
			lightSpecularContrib = pow( clamp(dot( norm.xyz, halfwayVec), 0.0f, 1.0f ), objectSpecularPower )
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
