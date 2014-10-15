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

GLFrame             viewFrame;
GLFrustum           viewFrustum;
GLTriangleBatch     torusBatch;
GLMatrixStack       modelViewMatrix;
GLMatrixStack       projectionMatrix;
GLGeometryTransform transformPipeline;

GLuint	ADSDissloveShader;	// The dissolving light shader
GLint	locAmbient;			// The location of the ambient color
GLint   locDiffuse;			// The location of the diffuse color
GLint   locSpecular;		// The location of the specular color
GLint	locLight;			// The location of the Light in eye coordinates
GLint	locMVP;				// The location of the ModelViewProjection matrix uniform
GLint	locMV;				// The location of the ModelView matrix uniform
GLint	locNM;				// The location of the Normal matrix uniform
GLint	locTexture;			// The location of the  texture uniform
GLint   locDissolveFactor;  // The location of the dissolve factor

GLuint	cloudTexture;		// The cloud texture texture object


/**
 * Window has changed size, or has just been created. In either case, we need to use the window 
 * dimensions to set the viewport and the projection matrix.
 */
void ChangeSize(int w, int h)
{
	glViewport(0, 0, w, h);

	viewFrustum.setPerspective(35.0f, float(w)/float(h), 1.0f, 100.0f);

	projectionMatrix.setMatrix(viewFrustum.GetProjectionMatrix());
	transformPipeline.setMatrixStacks(modelViewMatrix, projectionMatrix);
}

/**
 * This function does any needed initialization on the rendering context.
 * This is the first opportunity to do any OpenGL related tasks.
 */
void SetupRC()
{
	// Blue background
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f );

	glEnable(GL_DEPTH_TEST);

	shaderManager.init();

	viewFrame.MoveForward(4.0f);

	// create the torus
	gltCreateTorus(torusBatch, 0.8f, 0.25f, 52, 26);

	ADSDissloveShader = gltLoadShaderWithFileEx("Dissolve.vp", 
														"Dissolve.fp",
														3,
														GLT_ATTRIBUTE_VERTEX, "vVertex",
														GLT_ATTRIBUTE_NORMAL, "vNormal",
														GLT_ATTRIBUTE_TEXTURE0, "vTexCoords0");
	// in Dissolve.fp
	locAmbient = glGetUniformLocation(ADSDissloveShader, "ambientColor");
	locDiffuse = glGetUniformLocation(ADSDissloveShader, "diffuseColor");
	locSpecular = glGetUniformLocation(ADSDissloveShader, "specularColor");
	locLight = glGetUniformLocation(ADSDissloveShader, "vLightPosition");
	locTexture = glGetUniformLocation(ADSDissloveShader, "couldTexture");
	locDissolveFactor = glGetUniformLocation(ADSDissloveShader, "dissolveFactor");

	// in Dissolve.vp
	locMVP = glGetUniformLocation(ADSDissloveShader, "mvpMatrix");
	locMV = glGetUniformLocation(ADSDissloveShader, "mvMatrix");
	locNM = glGetUniformLocation(ADSDissloveShader, "normalMatrix");
	locLight = glGetUniformLocation(ADSDissloveShader, "vLightPosition");

	glGenTextures(1, &cloudTexture);
	glBindTexture(GL_TEXTURE_1D, cloudTexture);
	gltLoadTextureTGA("Clouds.tga", GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR, GL_CLAMP_TO_EDGE);
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

	modelViewMatrix.rotateTo(rotTimer.delta() * 10.0f, 0.0f, 1.0f, 0.0f);

	GLfloat vEyeLight[] = {-100.0f, 100.f, 100.f};
	GLfloat vAmbientColor[] = { 0.1f, 0.1f, 0.1f, 1.0f };
	GLfloat vDiffuseColor[] = { 0.1f, 1.0f, 0.1f, 1.0f };
	GLfloat vSpecularColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };

	glUseProgram(ADSDissloveShader);
	glUniform4fv(locAmbient, 1, vAmbientColor);
	glUniform4fv(locDiffuse, 1, vDiffuseColor);
	glUniform4fv(locSpecular, 1, vSpecularColor);

	glUniform3fv(locLight, 1, vEyeLight);

	glUniformMatrix4fv(locMVP, 1, GL_FALSE, transformPipeline.GetMVPMatrix());
	glUniformMatrix4fv(locMV, 1, GL_FALSE, transformPipeline.GetModelViewMatrix());
	glUniformMatrix3fv(locNM, 1, GL_FALSE, transformPipeline.GetNormalMatrix());

	glUniform1i(locTexture, 1);

	float fFactor = fmod(rotTimer.delta(), 10.0f) / 10.0f;
	glUniform1f(locDissolveFactor, fFactor);

	torusBatch.draw();

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
