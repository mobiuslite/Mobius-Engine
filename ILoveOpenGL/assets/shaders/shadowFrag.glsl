#version 420

out vec4 pixelColour;

void main()
{             
    gl_FragDepth = gl_FragCoord.z;
    pixelColour = vec4(1.0f, 0.0f, 0.0f, 1.0f);
}