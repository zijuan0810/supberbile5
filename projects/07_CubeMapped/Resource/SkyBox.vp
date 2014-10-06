#version 130

in vec4 vVertex;

uniform mat4 mvpMatrix;

// Texture coordinate to fragment program
varying vec3 vVaryingTexCoord;

void main()
{
	// Pass on the texture coordinates
	vVaryingTexCoord = normalize(vVertex.xyz);

	gl_Position = mvpMatrix * vVertex;
}