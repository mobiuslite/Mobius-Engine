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
layout (location = 4)out vec4 pixelSpecular;
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
uniform sampler2D texture_Emmision;

uniform sampler2D reticleTexture;

uniform sampler2D texLightpassColorBuf;
uniform sampler2D bloomMapColorBuf;
uniform sampler2D shadowMapColorBuf;

uniform bool bUseAlphaMask;
uniform sampler2D alphaMask;

uniform bool bUseMetallicMap;
uniform sampler2D metallicMap;

uniform bool bUseRoughMap;
uniform sampler2D roughMap;

uniform bool bUseNormalMap;
uniform sampler2D normalMap;

uniform bool bUseAO;
uniform sampler2D AOMap;

uniform bool bUseSkybox;
uniform bool bShowBloom;

uniform bool bIsPlane;

uniform bool bUseSkyboxReflections;

uniform vec4 textureRatios;

uniform vec2 screenWidthHeight;

//x = gamma;
//y = exposure;
//z use exposure based tonemapping > 0.5f
//w = bloom threshhold
uniform vec4 postprocessingVariables;
uniform vec4 cc;
uniform float saturation;

uniform float ambientPower;

uniform samplerCube skyBox;

const float PI = 3.14159f;

float TrowbridgeGGXNormalDist(vec3 N, vec3 H, float roughness);
float SchlickGGXGeometry(float NdotV, float roughness);
float SmithGeometry(vec3 N, vec3 V, vec3 L, float roughness);
vec3 schlickFresnel(float cosTheta, vec3 F0);

vec3 PBR(vec3 albedo, vec3 normal, vec3 worldPos, vec4 lightSpacePos, float roughness, float metallic);

float ShadowCalculation(vec4 fragPosLightSpace, vec3 normal, vec3 lightDir)
{
	// perform perspective divide, redundant but needed if in non-orthographic projection
	vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
	// transform to 0-1 range
	projCoords = projCoords * 0.5 + 0.5;

	float closestDepth = texture(shadowMapColorBuf, projCoords.xy).r;
	float currentDepth = projCoords.z;

	//Smooth Shadows
	float shadow = 0.0f;
	vec2 texelSize = 1.0 / textureSize(shadowMapColorBuf, 0);
	for (int x = -1; x <= 1; ++x)
	{
		for (int y = -1; y <= 1; ++y)
		{
			float pcfDepth = texture(shadowMapColorBuf, projCoords.xy + vec2(x, y) * texelSize).r;
			shadow += currentDepth - shadowBias > pcfDepth ? 1.0 : 0.0;
		}
	}
	shadow /= 9.0;

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

	float pixelRoughness = roughness;
	float pixelMetallic = metallic;

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

		const vec3 W = vec3(0.2125, 0.7154, 0.0721);
		vec3 intensity = vec3(dot(pixelColour.rgb, W));
		pixelColour.rgb = mix(intensity, pixelColour.rgb, saturation);

		pixelColour.rgb += cc.rgb * cc.a;

		vec3 reticlePixelColor = texture(reticleTexture, UVLookup).rgb;
		if (reticlePixelColor.r > 0.1f)
		{
			pixelColour.rgb = reticlePixelColor;
		}

		return;
	}

	if(passNumber == RENDER_PASS_1_LIGHT_PASS)
	{	
		vec2 UVLookup;
		UVLookup.x = gl_FragCoord.x / screenWidthHeight.x;
		UVLookup.y = gl_FragCoord.y / screenWidthHeight.y;

		//Sends the textures back to their fbo buffers, so we can display them for debugging purposes
		
		//w value = ao
		pixelMatColor = texture(texture_MatColor, UVLookup).rgba;
		float ao = pixelMatColor.w;

		//w value = roughness
		pixelNormal = texture(texture_Normal, UVLookup).rgba;
		float lightingRoughness = pixelNormal.w;

		//w value = is lit
		pixelWorldPos = texture(texture_WorldPos, UVLookup).rgba;

		//w value used for perspective divide
		pixelLightSpacePos = texture(texture_LightSpacePos, UVLookup).rgba;
		//pixelSpecular = texture(texture_Specular, UVLookup).rgba;

		//w value too metallic
		pixelEmmision = texture(texture_Emmision, UVLookup).rgba;
		float lightingMetallic = pixelEmmision.w;

		float depthValue = texture(shadowMapColorBuf, UVLookup).r;

		pixelSpecular = vec4(depthValue, depthValue, depthValue, 1.0f);
		//If not lit
		if(pixelWorldPos.w <= 0.1f)
		{
			pixelColour.rgb = pixelMatColor.rgb;
		}
		else
		{
			//pixelColour = calcualteLightContrib( pixelMatColor.rgb, pixelNormal.xyz, pixelWorldPos.xyz, pixelSpecular.rgba, pixelLightSpacePos);		
			//pixelColour.a = 1.0f;

			vec3 pbrColor = PBR(pixelMatColor.rgb, pixelNormal.xyz, pixelWorldPos.xyz, pixelLightSpacePos, lightingRoughness, lightingMetallic);
			
			
			vec3 ambient = ambientPower * pixelMatColor.rgb * ao;
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
		vec3 alphaMaskTexture = texture(alphaMask, vec2((fUVx2.x * tilingAndOffset.x) + tilingAndOffset.z, (fUVx2.y * tilingAndOffset.y) + tilingAndOffset.w)).rgb;

		float alphaValue = alphaMaskTexture.r;
		//pixelColour.a = alphaValue;

		if(alphaValue < 0.1f)
		{
			discard;
		}
	}

	if(bUseNormalMap)
	{
		vec3 normalMapTexture = texture(normalMap, vec2((fUVx2.x * tilingAndOffset.x) + tilingAndOffset.z, (fUVx2.y * tilingAndOffset.y) + tilingAndOffset.w)).rgb;
		normalMapTexture = normalMapTexture * 2.0f - 1.0f;

		normals.xyz = normalize(TBN * normalMapTexture);
	}

	if (bIsPlane)
	{
		if (!gl_FrontFacing)
		{
			normals.xyz = normals.xyz * -1.0f;
		}
	}

	if (bUseMetallicMap)
	{
		float mapTexture = texture(metallicMap, vec2((fUVx2.x * tilingAndOffset.x) + tilingAndOffset.z, (fUVx2.y * tilingAndOffset.y) + tilingAndOffset.w)).r;
		pixelMetallic = mapTexture * metallic;
	}

	if (bUseRoughMap)
	{
		float mapTexture = texture(roughMap, vec2((fUVx2.x * tilingAndOffset.x) + tilingAndOffset.z, (fUVx2.y * tilingAndOffset.y) + tilingAndOffset.w)).r;
		pixelRoughness = mapTexture * roughness;
	}

	if(bDebugMode && bDebugShowNormals)
	{
		pixelColour.rgb = normals.xyz;
		return;
	}

	//ST = UV
	if(textureRatios.x >  0.1f)
	{
		vec3 textureColour = (texture(texture_00, vec2((fUVx2.x * tilingAndOffset.x) + tilingAndOffset.z, (fUVx2.y * tilingAndOffset.y) + tilingAndOffset.w)).rgb * textureRatios.x);

		vertexDiffuseColour = vec4(textureColour, 1.0f);
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

	float aoValue = 1.0f;
	if (bUseAO)
	{
		aoValue = texture(AOMap, vec2((fUVx2.x * tilingAndOffset.x) + tilingAndOffset.z, (fUVx2.y * tilingAndOffset.y) + tilingAndOffset.w)).r;
	}

	pixelMatColor.w = aoValue;

	pixelNormal = vec4(normalize(normals.xyz), pixelRoughness);

	pixelWorldPos = vec4(fVertWorldLocation.xyz, 1.0f);
	pixelLightSpacePos = fLightSpacePos;

	pixelEmmision = vec4(emmision, pixelMetallic);

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

	vec3 finalColor = vec3(0.0);
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

		float NDF = TrowbridgeGGXNormalDist(N, H, roughness);
		float G = SmithGeometry(N, V, L, roughness);
		vec3 F = schlickFresnel(max(dot(V, H), 0.0), F0);

		vec3 kS = F;
		vec3 kD = (vec3(1.0) - kS) * (1.0 - metallic);

		//Calculate Cook-Torrance specular BRDF
		vec3 numerator = NDF * G * F;
		float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001;
		vec3 specular = numerator / denominator;

		float NdotL = max(dot(N, L), 0.0);

		float shadow = 0.0f;
		if (intLightType == DIRECTIONAL_LIGHT_TYPE)		// = 2
		{
			shadow = ShadowCalculation(lightSpacePos, N, L);
		}

		finalColor += ((kD * albedo / PI + specular) * radiance * NdotL) * (1.0f - shadow);
	}
	return finalColor;
}

float TrowbridgeGGXNormalDist(vec3 N, vec3 H, float roughness)
{
	float a = roughness * roughness;
	float aSquared = a * a;
	float NdotH = max(dot(N, H), 0.0);
	float NdotH2 = NdotH * NdotH;

	float num = aSquared;
	float denom = (NdotH2 * (aSquared - 1.0) + 1.0);
	denom = PI * denom * denom;

	return num / denom;
}

float SchlickGGXGeometry(float NdotV, float roughness)
{
	float r = (roughness + 1.0);
	float k = (r * r) / 8.0;

	float num = NdotV;
	float denom = NdotV * (1.0 - k) + k;

	return num / denom;
}
float SmithGeometry(vec3 N, vec3 V, vec3 L, float roughness)
{
	float NdotV = max(dot(N, V), 0.0);
	float NdotL = max(dot(N, L), 0.0);
	float ggx2 = SchlickGGXGeometry(NdotV, roughness);
	float ggx1 = SchlickGGXGeometry(NdotL, roughness);

	return ggx1 * ggx2;
}

vec3 schlickFresnel(float cosTheta, vec3 F0)
{
	return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}
