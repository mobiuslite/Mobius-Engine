#version 420

in vec4 vColour;
in vec4 vPosition;
in vec4 vNormal;		
in vec4 vUVx2;			
in vec4 vTangent;		
in vec4 vBiNormal;

uniform mat4 lightSpaceMatrix;
uniform mat4 matModel;

void main()
{
    gl_Position = lightSpaceMatrix * matModel * vPosition;
} 