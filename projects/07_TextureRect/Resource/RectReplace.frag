#version 140

out vec4 vFragColor;

uniform sampler2DRect rectangleImage;

smooth in vec2 vVaryingTexCoord;

void main()
{
	vFragColor = texture(rectangleImage, vVaryingTexCoord);
}