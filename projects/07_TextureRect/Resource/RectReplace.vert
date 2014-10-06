#version 130

in vec4 vVertex;
in vec2 vTexCoord;

uniform mat4 mvpMatrix;

// texture coordinate to fragment program
smooth out vec2 vVaryingTexCoord;

void main(void) 
{
	// pass on the texure coordinates
	vVaryingTexCoord = vTexCoord;

	// Don't forget to transform the geometry
	gl_Position = mvpMatrix * vVertex;
}