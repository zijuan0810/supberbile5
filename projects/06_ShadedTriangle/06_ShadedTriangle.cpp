#include <GLTools.h>
#include <GL/glut.h>
#include <GLShaderManager.h>

#include <GLFrustum.h>
#include <GLBatch.h>
#include <GLMatrixStack.h>
#include <GLGeometryTransform.h>
#include <StopWatch.h>

#include <math.h>
#include <stdio.h>

#include <iostream>

GLShaderManager	shaderManager;

GLBatch		triangleBatch;
GLint		myIndentityShader;

/**
 * Window has changed size, or has just been created. In either case, we need to use the window 
 * dimensions to set the viewport and the projection matrix.
 */
void ChangeSize(int w, int h)
{
	glViewport(0, 0, w, h);
}

/**
 * This function does any needed initialization on the rendering context.
 * This is the first opportunity to do any OpenGL related tasks.
 */
void SetupRC()
{
	// Blue background
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f );

	shaderManager.init();

	// Load up a triangle
	GLfloat vVerts[] = { 
		-0.5f, 0.0f, 0.0f, 
		0.5f, 0.0f, 0.0f,
		0.0f, 0.5f, 0.0f 
	};

	GLfloat vColors [] = { 
		1.0f, 0.0f, 0.0f, 1.0f,
		0.0f, 1.0f, 0.0f, 1.0f,
		0.0f, 0.0f, 1.0f, 1.0f 
	};
	triangleBatch.begin(GL_TRIANGLES, 3);
	triangleBatch.CopyVertexData3f(vVerts);
	triangleBatch.CopyColorData4f(vColors);
	triangleBatch.end();

	myIndentityShader = gltLoadShaderWithFileEx("ShadedIdentity.vsh", 
														"ShadedIdentity.fsh", 
														2, 
														GLT_ATTRIBUTE_VERTEX, "vVertex", 
														GLT_ATTRIBUTE_COLOR, "vColor");

	glProvokingVertex(GL_FIRST_VERTEX_CONVENTION);
}

/**
 * Respond to arrow keys by moving the camera frame of reference
 */
void SpecialKeys(int key, int x, int y)
{
}

/**
 * Respond to the key pressed
 */
void KeyPressFunc(unsigned char key, int x, int y)
{
	static int nToggle = 1;
	if(key == 32) {
		nToggle++;

		if(nToggle %2 == 0) {
			glProvokingVertex(GL_LAST_VERTEX_CONVENTION);
			glutSetWindowTitle("Provoking Vertex - Last Vertex - Press Space Bars");
		}
		else {
			glProvokingVertex(GL_FIRST_VERTEX_CONVENTION);
			glutSetWindowTitle("Provoking Vertex - First Vertex - Press Space Bars");
		}

		glutPostRedisplay();
	}
}

/**
 * Reset flags as appropriate in response to menu selections
 */
void ProcessMenu(int value)
{
}

/**
 * Called to draw scene
 */
void RenderScene(void)
{
	// Clear the window with current clearing color
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	glUseProgram(myIndentityShader);
	triangleBatch.draw();

	// Perform the buffer swap to display back buffer
	glutSwapBuffers();
}

/**
 * Cleanup... such as deleting texture objects
 */
void OnExit()
{
	glDeleteProgram(myIndentityShader);
}

/**
 * Main entry point for GLUT based programs
 */
int main(int argc, char* argv[])
{
	gltSetWorkingDirectory(argv[0]);

	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH | GLUT_STENCIL);
	glutInitWindowSize(800, 600);
	glutCreateWindow("Triangle");

	gltShowVersionInfo();

	// Create the menu
	glutCreateMenu(ProcessMenu);
	glutAddMenuEntry("Btn1",1);
	glutAddMenuEntry("Btn2",2);
	glutAttachMenu(GLUT_RIGHT_BUTTON);

	glutReshapeFunc(ChangeSize);
	glutDisplayFunc(RenderScene);
	glutSpecialFunc(SpecialKeys);
	glutKeyboardFunc(KeyPressFunc);

	GLenum err = glewInit();
	if (GLEW_OK != err) {
		fprintf(stderr, "GLEW Error: %s\n", glewGetErrorString(err));
		return 1;
	}

	SetupRC();

	glutMainLoop();

	OnExit();

	return 0;
}
