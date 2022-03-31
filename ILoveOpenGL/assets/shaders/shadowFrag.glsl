#version 420

in vec4 fVertexColour;
in vec4 fVertWorldLocation;
in vec4 fNormal;
in vec4 fUVx2;
in vec4 fVertPosition;
in mat3 TBN;

out vec4 pixelColour;

void main()
{             
    //gl_FragDepth = gl_FragCoord.z;
    //pixelColour = vec4(gl_FragCoord.z, gl_FragCoord.z, gl_FragCoord.z, 1.0f);
}