#version 130

uniform sampler2D colorMap;

out vec4 vFragColor;

smooth in vec2 vVaryingTexCoords;

void main()
{
	vFragColor = texture(colorMap, vVaryingTexCoords.st);
}
