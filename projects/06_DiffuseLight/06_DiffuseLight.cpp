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

GLFrame			viewFrame;
GLFrustum		viewFrustum;
GLTriangleBatch	sphereBatch;
GLMatrixStack		modelViewMatrix;
GLMatrixStack		projectionMatrix;

GLGeometryTransform transformPipeline;

GLuint	diffuseLightShader;	// The diffuse light shader
GLint	locColor;			// The location of the diffuse color
GLint	locLight;			// The location of the light in eye coordinates
GLint	locMVP;				// The location of the ModelViewProjection matrix uniform
GLint	locMV;				// The location of the ModelView matrix uniform
GLint	locNM;			// The location of the Normal matrix uniform


/**
 * Window has changed size, or has just been created. In either case, we need to use the window 
 * dimensions to set the viewport and the projection matrix.
 */
void ChangeSize(int w, int h)
{
	glViewport(0, 0, w, h);

	viewFrustum.SetPerspective(35.0f, (float)w/(float)w, 1.0f, 100.0f);

	projectionMatrix.setMatrix(viewFrustum.GetProjectionMatrix());
	transformPipeline.SetMatrixStacks(modelViewMatrix, projectionMatrix);
}

/**
 * This function does any needed initialization on the rendering context.
 * This is the first opportunity to do any OpenGL related tasks.
 */
void SetupRC()
{
	// Blue background
	glClearColor(0.3f, 0.3f, 0.3f, 1.0f );

	shaderManager.init();

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);

	viewFrame.MoveForward(4.0f);

	// Make the sphere
	//gltMakeSphere(sphereBatch, 1.0f, 26, 13);
	gltMakeTorus(sphereBatch, 0.4f, 0.15f, 40, 20);

	diffuseLightShader = shaderManager.LoadShaderPairWithAttributes("DiffuseLight.vsh", 
		"DiffuseLight.fsh", 2, 
		GLT_ATTRIBUTE_VERTEX, "vVertex", 
		GLT_ATTRIBUTE_NORMAL, "vNormal");
	locColor  = glGetUniformLocation(diffuseLightShader, "diffuseColor");
	locLight  = glGetUniformLocation(diffuseLightShader, "vLightPosition");
	locMVP	 = glGetUniformLocation(diffuseLightShader, "mvpMatrix");
	locMV	 = glGetUniformLocation(diffuseLightShader, "mvMatrix");
	locNM	 = glGetUniformLocation(diffuseLightShader, "normalMatrix");
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

	GLfloat vRed[] = { 1.0f, 0.0f, 0.0f, 1.0f };
	shaderManager.useStockShader(GLT_SHADER_IDENTITY, vRed);

	modelViewMatrix.push(viewFrame);

	modelViewMatrix.Rotate(rotTimer.GetElapsedSeconds() * 10.0f, 0.0f, 1.0f, 0.0f);

	GLfloat vEyeLight[] = {-100.0f, 100.0f, 100.0f};
	GLfloat vDiffuseColor[] = {0.0f, 0.0f, 1.0f, 1.0f};

	glUseProgram(diffuseLightShader);
	glUniform4fv(locColor, 1, vDiffuseColor);
	glUniform3fv(locLight, 1, vEyeLight);
	glUniformMatrix4fv(locMVP, 1, GL_FALSE, transformPipeline.GetMVPMatrix());
	glUniformMatrix4fv(locMV, 1, GL_FALSE, transformPipeline.GetModelViewMatrix());
	glUniformMatrix3fv(locNM, 1, GL_FALSE, transformPipeline.GetNormalMatrix());

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
