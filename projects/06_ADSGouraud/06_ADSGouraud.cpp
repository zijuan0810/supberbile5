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

GLFrame				viewFrame;
GLFrustum			viewFrustum;
GLTriangleBatch		sphereBatch;
GLMatrixStack		modelViewMatrix;
GLMatrixStack		projectionMatrix;

GLGeometryTransform	transformPieline;

GLuint	ADSLightShader;		// The diffuse light shader
GLint	locAmbient;			// The location of the ambient color
GLint	locDiffuse;			// The location of the diffuse color;
GLint	locSpecular;		// The location of the specular color;
GLint	locLight;			// The location of the Light in eye coordinates
GLint	locMVP;				// The location of the ModelViewProjection matrix uniform
GLint	locMV;				// The location of the ModelView matrix
GLint	locNM;				// The location of the Normal matrix unifom


/**
 * Window has changed size, or has just been created. In either case, we need to use the window 
 * dimensions to set the viewport and the projection matrix.
 */
void ChangeSize(int w, int h)
{
	glViewport(0, 0, w, h);

	viewFrustum.SetPerspective(35.0f, float(w)/float(h), 1.0f, 100.0f);

	projectionMatrix.setMatrix(viewFrustum.GetProjectionMatrix());
	transformPieline.SetMatrixStacks(modelViewMatrix, projectionMatrix);
}

/**
 * This function does any needed initialization on the rendering context.
 * This is the first opportunity to do any OpenGL related tasks.
 */
void SetupRC()
{
	// Blue background
	glClearColor(0.0f, 0.0f, 1.0f, 1.0f );

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);

	viewFrame.MoveForward(4.0f);

	// Make the sphere
	gltMakeSphere(sphereBatch, 1.0f, 26, 13);

	// GLSL ���ݻ�ȡ
	ADSLightShader = shaderManager.LoadShaderPairWithAttributes("ADSGouraud.vp", 
																"ADSGouraud.fp",
																2,
																GLT_ATTRIBUTE_VERTEX,
																"vVertex",
																GLT_ATTRIBUTE_NORMAL,
																"vNormal");
	locAmbient = glGetUniformLocation(ADSLightShader, "ambientColor");
	locDiffuse = glGetUniformLocation(ADSLightShader, "diffuseColor");
	locSpecular = glGetUniformLocation(ADSLightShader, "specularColor");

	locLight = glGetUniformLocation(ADSLightShader, "vLightPosition");
	locMVP = glGetUniformLocation(ADSLightShader, "mvpMatrix");
	locMV = glGetUniformLocation(ADSLightShader, "mvMatrix");
	locNM = glGetUniformLocation(ADSLightShader, "normalMatrix");

	shaderManager.init();
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
	static CStopWatch rotTimer;

	// Clear the window with current clearing color
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	modelViewMatrix.push(viewFrame);

	modelViewMatrix.Rotate(10.0f * rotTimer.GetElapsedSeconds(), 0.0f, 1.0f, 0.0f);

	GLfloat vEyeLight[] = {-100.0f, 100.0f, 100.0f};
	GLfloat vAmbientColor[] = {0.1f, 0.1f, 0.1f, 1.0f};
	GLfloat vDiffuseColor[] = {0.0f, 0.0f, 1.0f, 1.0f};
	GLfloat vSpecularColor[] = {1.0f, 1.0f, 1.0f, 1.0f};

	glUseProgram(ADSLightShader);

	glUniform4fv(locAmbient, 1, vAmbientColor);
	glUniform4fv(locDiffuse, 1, vDiffuseColor);
	glUniform4fv(locSpecular, 1, vSpecularColor);

	glUniform3fv(locLight, 1, vEyeLight);
	glUniformMatrix4fv(locMVP, 1, GL_FALSE, transformPieline.GetMVPMatrix());
	glUniformMatrix4fv(locMV, 1, GL_FALSE, transformPieline.GetModelViewMatrix());
	glUniformMatrix3fv(locNM, 1, GL_FALSE, transformPieline.GetNormalMatrix());

	sphereBatch.draw();

	modelViewMatrix.pop();

	// Perform the buffer swap to display back buffer
	glutSwapBuffers();
	glutPostRedisplay();
}

/**
 * Cleanup... such as deleting texture objects
 */
void OnExit()
{
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
