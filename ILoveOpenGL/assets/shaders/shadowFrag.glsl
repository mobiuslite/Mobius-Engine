#version 420
in vec4 fUVx2;

out vec4 pixelColour;

uniform bool bUseAlphaMask;
uniform sampler2D alphaMask;

void main()
{             

	//pixelColour = vec4(gl_FragCoord.z, gl_FragCoord.z, gl_FragCoord.z, 1.0f);
}