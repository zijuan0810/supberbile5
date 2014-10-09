// Starfield Shader
// Vertex Shader
// Richard S. Wright Jr.
// OpenGL SuperBible
#version 130

// Incoming per vertex... position
in vec4 vVertex;

uniform mat4 mvpMatrix;

void main(void) 
{
    // Don't forget to transform the geometry!
    gl_Position = mvpMatrix * vVertex;
}
