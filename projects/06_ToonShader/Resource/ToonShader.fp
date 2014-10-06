#version 130

uniform sampler1D colorTable;
out vec4 vFragColor;

smooth in float textureCoordinate;

void main()
{
	vFragColor = texture(colorTable, textureCoordinate);
}