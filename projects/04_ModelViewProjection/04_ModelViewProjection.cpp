#include <iostream>

#include <GLTools.h>
#include <GL/glut.h>
#include <GLShaderManager.h>

#include "GLMatrixStack.h"
#include "GLFrame.h"
#include "GLFrustum.h"
#include "GLGeometryTransform.h"
#include "GLBatch.h"
#include "StopWatch.h"

GLShaderManager	shaderManager;

GLFrustum	viewFrustum;

GLTriangleBatch torusBatch;


/**
 * Window has changed size, or has just been created. In either case, we need to use the window 
 * dimensions to set the viewport and the projection matrix.
 */
void ChangeSize(int w, int h)
{
	glViewport(0, 0, w, h);

	viewFrustum.SetPerspective(35.0f, (float)w/(float)h, 1.0f, 1000.0f);
}

/**
 * This function does any needed initialization on the rendering context.
 * This is the first opportunity to do any OpenGL related tasks.
 */
void SetupRC()
{
	// Blue background
	glClearColor(0.8f, 0.8f, 0.8f, 1.0f );

	shaderManager.init();

	glEnable(GL_DEPTH_TEST);

	// This makes a torus
	gltCreateTorus(torusBatch, 0.4f, 0.15f, 30, 30);

	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
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
	glutPostRedisplay();
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

	// Set up time based animation
	static CStopWatch rotTimer;
	float yRot = rotTimer.GetElapsedSeconds() * 60.0f;

	std::cout << yRot << std::endl;

	M3DMatrix44f mTranslate, mRotate, mModelview, mModelViewProjection;
	m3dTranslationMatrix44(mTranslate, 0.0f, 0.0f, -2.5f);
	m3dRotationMatrix44(mRotate, m3dDegToRad(yRot), 0.0f, 1.0f, 0.0f);

	// Add the rotation to the translation, store the result in modelview
	m3dMatrixMultiply44(mModelview, mTranslate, mRotate);

	// Add the modelview matrix to the projection matrix, 
	// the final matrix is the ModelViewProjection matrix.
	m3dMatrixMultiply44(mModelViewProjection, viewFrustum.GetProjectionMatrix(), mModelview);

	// Pass this completed matrix to the shader, and render the torus
	GLfloat vRed[] = { 1.0f, 0.0f, 0.0f, 1.0f };
	shaderManager.useStockShader(GLT_SHADER_FLAT, mModelViewProjection, vRed);
	torusBatch.draw();

	// Perform the buffer swap to display back buffer
	glutSwapBuffers();
	glutPostRedisplay();
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
	return 0;
}
