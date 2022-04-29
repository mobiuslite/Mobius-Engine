#version 420

layout(location = 0) in vec4 vColour;
layout(location = 1) in vec4 vPosition;
layout(location = 2) in vec4 vNormal;
layout(location = 3) in vec4 vUVx2;
layout(location = 4) in vec4 vTangent;
layout(location = 5) in vec4 vBiNormal;
layout(location = 6) in vec4 vInstancedOffset;

out vec4 fUVx2;
out vec4 fNormal;
out vec4 fVertWorldLocation;

uniform mat4 lightSpaceMatrix;
uniform mat4 matModel;

uniform bool bUseWind;
uniform float windTime;
uniform float windSize;
uniform float windStrength;

uniform vec3 windDirection;

uniform sampler2D windMap;

uniform bool bUseInstancedRendering;

void main()
{
    vec4 vertPosition = vPosition;

    if (bUseInstancedRendering)
    {
        vertPosition += vInstancedOffset;
    }

    fVertWorldLocation = matModel * vertPosition;
    fVertWorldLocation.w = 1.0f;

    if (bUseWind)
    {
        float windSample = texture(windMap, (fVertWorldLocation.xz + vec2(windTime)) * windSize).r;
        //Makes 0-1 range into -1 - 1 range
        windSample = windSample * 2.0f - 1.0f;
        windSample *= vUVx2.y * windStrength;

        vertPosition += vec4(normalize(windDirection) * windSample, 1.0f);
    }

    gl_Position = lightSpaceMatrix * matModel * vertPosition;
    fUVx2 = vUVx2;
    fNormal = normalize(vNormal);
} 