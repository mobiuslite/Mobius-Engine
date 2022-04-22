#version 420

in vec4 fVertexColour;			// The vertex colour from the original model
in vec4 fVertWorldLocation;
in vec4 fNormal;
in vec4 fUVx2;
in vec4 fVertPosition;
in mat3 TBN;

layout(location = 0)out vec4 pixelColour;

uniform sampler2D bloomMap;
uniform vec2 screenWidthHeight;

uniform float bloomSize;
  
uniform bool horizontal;
float weight[5] = float[] (0.227027, 0.1945946, 0.1216216, 0.054054, 0.016216);

void main()
{         
    vec2 UVLookup;
	UVLookup.x = gl_FragCoord.x / screenWidthHeight.x;
	UVLookup.y = gl_FragCoord.y / screenWidthHeight.y;
    
    //Get size of a pixel
    vec2 texOffset = 1.0 / textureSize(bloomMap, 0) * bloomSize;
    vec3 result = texture(bloomMap, UVLookup).rgb * weight[0];

    if(horizontal)
    {
        for(int i = 1; i < 5; ++i)
        {
            result += texture(bloomMap, UVLookup + vec2(texOffset.x * i, 0.0)).rgb * weight[i];
            result += texture(bloomMap, UVLookup - vec2(texOffset.x * i, 0.0)).rgb * weight[i];
        }
    }
    else
    {
        for(int i = 1; i < 5; ++i)
        {
            result += texture(bloomMap, UVLookup + vec2(0.0, texOffset.y * i)).rgb * weight[i];
            result += texture(bloomMap, UVLookup - vec2(0.0, texOffset.y * i)).rgb * weight[i];
        }
    }
    pixelColour = vec4(result, 1.0);
}