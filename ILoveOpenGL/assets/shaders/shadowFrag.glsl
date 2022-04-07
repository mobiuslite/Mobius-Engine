#version 420

in vec4 fUVx2;
in vec4 fNormal;
in vec4 fVertWorldLocation;

out vec4 pixelColour;

uniform bool bUseAlphaMask;
uniform sampler2D alphaMask;

void main()
{           

	if (bUseAlphaMask)
	{
		vec3 alphaMaskTexture = texture(alphaMask, fUVx2.xy).rgb;

		float alphaValue = alphaMaskTexture.r;
		//pixelColour.a = alphaValue;

		if (alphaValue < 0.001f)
		{
			discard;
		}
	}
    //gl_FragDepth = gl_FragCoord.z;
    //pixelColour = vec4(gl_FragCoord.z, gl_FragCoord.z, gl_FragCoord.z, 1.0f);
}