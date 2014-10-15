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

GLMatrixStack		modelViewMatrix;
GLMatrixStack		projectionMatrix;
GLFrustum			viewFrustum;
GLGeometryTransform	transformPipeline;

GLBatch		floorBatch;
GLBatch		ceilingBatch;
GLBatch		leftWallBatch;
GLBatch		rightWallBatch;

GLfloat		viewZ = -65.0f;

// Texture ojbects;
#define	TEXTURE_BRICK	0
#define	TEXTURE_FLOOR	1
#define	TEXTURE_CEILING	2

#define	TEXTURE_COUNT	3

GLuint	arrTextures[TEXTURE_COUNT];

const char* szTextureFiles[TEXTURE_COUNT] = {"brick.tga", "floor.tga", "ceiling.tga"};


/**
 * Window has changed size, or has just been created. In either case, we need to use the window 
 * dimensions to set the viewport and the projection matrix.
 */
void ChangeSize(int w, int h)
{
	// Prevent a divide by zero
	if ( h == 0 ) {
		h = 1;
	}

	// Set viewport to window dimensions
	glViewport(0, 0, w, h);

	GLfloat fAspect = (GLfloat)w/(GLfloat)h;
	// Produce the perspective projection
	viewFrustum.setPerspective(80.0f, fAspect, 1.0f, 120.0f);
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
	glClearColor(0.0f, 0.0f, 1.0f, 1.0f );

	shaderManager.init();

	// Load texure
	glGenTextures(TEXTURE_COUNT, arrTextures);
	for (int i = 0; i < TEXTURE_COUNT; i++) {
		GLint iWidth, iHeight, iComponents;
		GLenum eFormat;
		// Bind to next texture object
		glBindTexture(GL_TEXTURE_2D, arrTextures[i]);
		// Load texture
		GLbyte* pBytes = gltReadTGABits(szTextureFiles[i], &iWidth, &iHeight, &iComponents, &eFormat);
		// set filter and wrap modes
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexImage2D(GL_TEXTURE_2D, 0, iComponents, iWidth, iHeight, 0, eFormat, GL_UNSIGNED_BYTE,
					pBytes);
		glGenerateMipmap(GL_TEXTURE_2D);
		// Don't need original texure data any more
		free(pBytes);
	}

	// Build Geometry
	floorBatch.begin(GL_TRIANGLE_STRIP, 28, 1);	for(GLfloat i = 60.0f; i >= 0.0f; i -= 10.0f) {		floorBatch.MultiTexCoord2f(0, 0.0f, 0.0f);		floorBatch.Vertex3f(-10.0f, -10.0f, i);
		floorBatch.MultiTexCoord2f(0, 1.0f, 0.0f);		floorBatch.Vertex3f(10.0f, -10.0f, i);

		floorBatch.MultiTexCoord2f(0, 0.0f, 1.0f);		floorBatch.Vertex3f(-10.0f, -10.0f, i - 10.0f);

		floorBatch.MultiTexCoord2f(0, 1.0f, 1.0f);		floorBatch.Vertex3f(10.0f, -10.0f, i - 10.0f);	}	floorBatch.end();
	ceilingBatch.begin(GL_TRIANGLE_STRIP, 28, 1);	for ( GLfloat i = 60.0f; i >= 0.0f; i -=10.0f) {		ceilingBatch.MultiTexCoord2f(0, 0.0f, 1.0f);		ceilingBatch.Vertex3f(-10.0f, 10.0f, i - 10.0f);
		ceilingBatch.MultiTexCoord2f(0, 1.0f, 1.0f);		ceilingBatch.Vertex3f(10.0f, 10.0f, i - 10.0f);
		ceilingBatch.MultiTexCoord2f(0, 0.0f, 0.0f);		ceilingBatch.Vertex3f(-10.0f, 10.0f, i);
		ceilingBatch.MultiTexCoord2f(0, 1.0f, 0.0f);		ceilingBatch.Vertex3f(10.0f, 10.0f, i);	}	ceilingBatch.end();
	leftWallBatch.begin(GL_TRIANGLE_STRIP, 28, 1);	for (GLfloat i = 60.0f; i >= 0.0f; i -=10.0f) {		leftWallBatch.MultiTexCoord2f(0, 0.0f, 0.0f);		leftWallBatch.Vertex3f(-10.0f, -10.0f, i);
		leftWallBatch.MultiTexCoord2f(0, 0.0f, 1.0f);		leftWallBatch.Vertex3f(-10.0f, 10.0f, i);
		leftWallBatch.MultiTexCoord2f(0, 1.0f, 0.0f);		leftWallBatch.Vertex3f(-10.0f, -10.0f, i - 10.0f);
		leftWallBatch.MultiTexCoord2f(0, 1.0f, 1.0f);		leftWallBatch.Vertex3f(-10.0f, 10.0f, i - 10.0f);	}	leftWallBatch.end();
	rightWallBatch.begin(GL_TRIANGLE_STRIP, 28, 1);	for (GLfloat i = 60.0f; i >= 0.0f; i -=10.0f) {		rightWallBatch.MultiTexCoord2f(0, 0.0f, 0.0f);		rightWallBatch.Vertex3f(10.0f, -10.0f, i);
		rightWallBatch.MultiTexCoord2f(0, 0.0f, 1.0f);		rightWallBatch.Vertex3f(10.0f, 10.0f, i);
		rightWallBatch.MultiTexCoord2f(0, 1.0f, 0.0f);		rightWallBatch.Vertex3f(10.0f, -10.0f, i - 10.0f);
		rightWallBatch.MultiTexCoord2f(0, 1.0f, 1.0f);		rightWallBatch.Vertex3f(10.0f, 10.0f, i - 10.0f);	}	rightWallBatch.end();}

/**
 * Respond to arrow keys by moving the camera frame of reference
 */
void SpecialKeys(int key, int x, int y)
{
	if ( key == GLUT_KEY_UP ) {
		viewZ += 0.5f;
	}
	else if ( key == GLUT_KEY_DOWN ) {
		viewZ -= 0.5f;
	}

	glutPostRedisplay();
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
	for (int i = 0; i < TEXTURE_COUNT; i++) {
		glBindTexture(GL_TEXTURE_2D, arrTextures[i]);
		switch (value) {
		case 0:
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			break;
		case 1:
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			break;
		case 2:
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
			break;
		case 3:
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
			break;
		case 4:
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
			break;
		case 5:
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
			break;
		case 6:
			{
				GLfloat fLargest = 0;
				glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &fLargest);	// 获取各向异性支持的数量
				glTexParameteri(GL_TEXTURE_2D, GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, fLargest);
			}
			break;
		case 7:
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, 1);
		}
	}

	// Trigger redraw
	glutPostRedisplay();
}

/**
 * Called to draw scene
 */
void RenderScene(void)
{
	// Clear the window with current clearing color
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	modelViewMatrix.push();

	modelViewMatrix.moveTo(0.0f, 0.0f, viewZ);
	shaderManager.useStockShader(GLT_SHADER_TEXTURE_REPLACE, 
								transformPipeline.GetMVPMatrix(),
								0);
	glBindTexture(GL_TEXTURE_2D, arrTextures[TEXTURE_FLOOR]);
	floorBatch.draw();

	glBindTexture(GL_TEXTURE_2D, arrTextures[TEXTURE_CEILING]);
	ceilingBatch.draw();

	glBindTexture(GL_TEXTURE_2D, arrTextures[TEXTURE_BRICK]);
	leftWallBatch.draw();
	rightWallBatch.draw();

	modelViewMatrix.pop();

	GLfloat vRed[] = { 1.0f, 0.0f, 0.0f, 1.0f };
	shaderManager.useStockShader(GLT_SHADER_IDENTITY, vRed);

	// Perform the buffer swap to display back buffer
	glutSwapBuffers();
}

/**
 * Cleanup job
 */
void CleanupWork()
{
	glDeleteTextures(TEXTURE_COUNT, arrTextures);
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
	glutAddMenuEntry("GL_NEAREST", 0);
	glutAddMenuEntry("GL_LINEAR", 1);
	glutAddMenuEntry("GL_NEAREST_MIPMAP_NEAREST", 2);
	glutAddMenuEntry("GL_NEAREST_MIPMAP_LINEAR", 3);
	glutAddMenuEntry("GL_LINEAR_MIPMAP_NEAREST", 4);
	glutAddMenuEntry("GL_LINEAR_MIPMAP_NEAREST", 5);
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

	gltShowVersionInfo();

	SetupRC();

	glutMainLoop();
	return 0;
}
