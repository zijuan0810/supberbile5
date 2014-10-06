// Fragment Shader

out vec4 vFragColor;

uniform samplerCube cubeMap;

smooth in vec3 vVaryingTexCoord;

void main()
{
	vFragColor = texture(cubeMap, vVaryingTexCoord);
}