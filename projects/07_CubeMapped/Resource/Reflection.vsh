// Vertex Shader

#version 130

in vec4 vVertex;
in vec3 vNormal;

uniform mat4 mvpMatrix;
uniform mat4 mvMatrix;
uniform mat3 normalMatrix;
uniform mat4 mInverseCamera;

// texture cooordinate to fragment progam
smooth out vec3 vVaryingTexCoord;


void main()
{
	// normal in eye space
	vec3 vEyeNormal = normalMatrix * vNormal;

	// vertex position in eye space
	vec4 vVert4 = mvMatrix * vVertex;
	vec3 vEyeVertex = normalize(vVert4.xyz / vVert4.w);

	// get reflected vector
	vec4 vCoords = vec4(reflect(vEyeVertex, vEyeNormal), 1.0);

	// rorate by flipped camera
	vCoords = mInverseCamera * vCoords;
	vVaryingTexCoord.xyz = normalize(vCoords.xyz);

	gl_Position = mvpMatrix * vVertex;
}