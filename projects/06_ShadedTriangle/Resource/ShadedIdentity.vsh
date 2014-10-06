// The ShadedIdentity Shader
// Vertex Shader
// Richard S. Wright Jr.
// OpenGL SuperBible
#version 130

in vec4 vColor;
in vec4 vVertex;

// flat关键字不进行插值
flat out vec4 vVaryingColor;

void main(void) 
{ 
    vVaryingColor = vColor;
    gl_Position = vVertex;
}