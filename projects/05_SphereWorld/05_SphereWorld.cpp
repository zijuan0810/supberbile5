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

#define NUM_SPHERES 50
GLFrame spheres[NUM_SPHERES];

GLShaderManager	shaderManager;

GLMatrixStack		modelViewMatrix;		// Modelview Matrix
GLMatrixStack		projectionMatrix;		// Projection Matrix
GLFrustum			viewFrustum;			// View Frustum
GLGeometryTransform	transformPipeline;		// Geometry Transform Pipeline
GLFrame				cameraFrame;			// Camera frame

GLTriangleBatch		torusBatch;
GLTriangleBatch		sphereBatch;
GLBatch				floorBatch;

GLuint				uiTextures[3];

void _DrawSongAndDance(GLfloat yRot)
{
	static GLfloat vWhite[] = {1.0f, 1.0f, 1.0f, 1.0f};
	static GLfloat vLightPos[] = {0.0f, 3.0f, 0.0f, 1.0f};

	// Get the light position in eye space
	M3DVector4f vLightTransformed;
	M3DMatrix44f mCamera;
	modelViewMatrix.GetMatrix(mCamera);
	m3dTransformVector4(vLightTransformed, vLightPos, mCamera);

	// draw the light source
	modelViewMatrix.push();
	modelViewMatrix.Translatev(vLightPos);
	shaderManager.useStockShader(GLT_SHADER_FLAT, transformPipeline.GetMVPMatrix(),
								vWhite);
	sphereBatch.draw();
	modelViewMatrix.pop();

	glBindTexture(GL_TEXTURE_2D, uiTextures[2]);
	for (int i = 0; i < NUM_SPHERES; ++i) {
		modelViewMatrix.push();
		modelViewMatrix.MultMatrix(spheres[i]);
		shaderManager.useStockShader(GLT_SHADER_TEXTURE_POINT_LIGHT_DIFF,
									modelViewMatrix.GetMatrix(),
									transformPipeline.GetProjectionMatrix(),
									vLightTransformed,
									vWhite,
									0);
		sphereBatch.draw();
		modelViewMatrix.pop();
	}

	// Song and dance
	modelViewMatrix.Translate(0.0f, 0.2f, -2.5f);
	modelViewMatrix.push();	// Saves the translated origin
	modelViewMatrix.Rotate(yRot, 0.0f, 1.0f, 0.0f);

	// draw stuff relative to the camera
	glBindTexture(GL_TEXTURE_2D, uiTextures[1]);
	shaderManager.useStockShader(GLT_SHADER_TEXTURE_POINT_LIGHT_DIFF,
								modelViewMatrix.GetMatrix(),
								transformPipeline.GetProjectionMatrix(),
								vLightTransformed,
								vWhite,
								0);
	torusBatch.draw();
	modelViewMatrix.pop();

	modelViewMatrix.Rotate(-2.0f*yRot, 0.0f, 1.0f, 0.0f);
	modelViewMatrix.Translate(0.8f, 0.0f, 0.0f);

	glBindTexture(GL_TEXTURE_2D, uiTextures[2]);
	shaderManager.useStockShader(GLT_SHADER_TEXTURE_POINT_LIGHT_DIFF,
								modelViewMatrix.GetMatrix(),
								transformPipeline.GetProjectionMatrix(),
								vLightTransformed,
								vWhite,
								0);
	sphereBatch.draw();
}

bool LoadTGATexture(const char *szFileName, GLenum minFilter, GLenum magFilter, GLenum wrapMode)
{
	GLbyte *pBits;
	int nWidth, nHeight, nComponents;
	GLenum eFormat;

	// Read the texture bits
	pBits = gltReadTGABits(szFileName, &nWidth, &nHeight, &nComponents, &eFormat);
	if(pBits == NULL) {
		return false;
	}

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapMode);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapMode);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minFilter);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, magFilter);

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_COMPRESSED_RGB, nWidth, nHeight, 0, eFormat,
				GL_UNSIGNED_BYTE, pBits);
	free(pBits);

	if (minFilter == GL_LINEAR_MIPMAP_LINEAR || minFilter == GL_LINEAR_MIPMAP_NEAREST || 
		minFilter == GL_NEAREST_MIPMAP_LINEAR || minFilter == GL_NEAREST_MIPMAP_NEAREST) {
		glGenerateMipmap(GL_TEXTURE_2D);
	}

	return true;
}


/**
 * Window has changed size, or has just been created. In either case, we need to use the window 
 * dimensions to set the viewport and the projection matrix.
 */
void ChangeSize(int w, int h)
{
	glViewport(0, 0, w, h);
	transformPipeline.SetMatrixStacks(modelViewMatrix, projectionMatrix);

	viewFrustum.SetPerspective(35.0f, (float)w/(float)h, 1.0f, 100.0f);
	projectionMatrix.setMatrix(viewFrustum.GetProjectionMatrix());
	modelViewMatrix.identity();
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

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);

	// This makes a torus
	gltMakeTorus(torusBatch, 0.4f, 0.15f, 40, 20);
	// This makes a sphere
	gltMakeSphere(sphereBatch, 0.1f, 26, 13);

	// Make the solid ground
	GLfloat texSize = 10.0f;
	floorBatch.Begin(GL_TRIANGLE_FAN, 4, 1);
	floorBatch.MultiTexCoord2f(0, 0.0f, 0.0f);
	floorBatch.Vertex3f(-20.0f, -0.41f, 20.0f);

	floorBatch.MultiTexCoord2f(0, texSize, 0.0f);
	floorBatch.Vertex3f(20.0f, -0.41f, 20.0f);

	floorBatch.MultiTexCoord2f(0, texSize, texSize);
	floorBatch.Vertex3f(20.0f, -0.41f, -20.0f);

	floorBatch.MultiTexCoord2f(0, 0.0f, texSize);
	floorBatch.Vertex3f(-20.0f, -0.41f, -20.0f);
	floorBatch.End();

	// Make 3 texture objects
	glGenTextures(3, uiTextures);

	// Load the Marble
	glBindTexture(GL_TEXTURE_2D, uiTextures[0]);
	LoadTGATexture("marble.tga", GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR, GL_REPEAT);

	// Load Mars
	glBindTexture(GL_TEXTURE_2D, uiTextures[1]);
	LoadTGATexture("marslike.tga", GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR, GL_CLAMP_TO_EDGE);

	// Load Moon
	glBindTexture(GL_TEXTURE_2D, uiTextures[2]);
	LoadTGATexture("moonlike.tga", GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR, GL_CLAMP_TO_EDGE);

	// Randomly place the spheres
	for (int i = 0; i < NUM_SPHERES; i++) {
		GLfloat x = ((GLfloat)((rand() % 400) - 200) * 0.1f);
		GLfloat z = ((GLfloat)((rand() % 400) - 200) * 0.1f);
		spheres[i].SetOrigin(x, 0.0f, z);
	}
}

/**
 * Respond to arrow keys by moving the camera frame of reference
 */
void SpecialKeys(int key, int x, int y)
{
	float linear = 0.1f;
	float angular = float(m3dDegToRad(5.0f));

	if(key == GLUT_KEY_UP)
		cameraFrame.MoveForward(linear);

	if(key == GLUT_KEY_DOWN)
		cameraFrame.MoveForward(-linear);

	if(key == GLUT_KEY_LEFT)
		cameraFrame.RotateWorld(angular, 0.0f, 1.0f, 0.0f);

	if(key == GLUT_KEY_RIGHT)
		cameraFrame.RotateWorld(-angular, 0.0f, 1.0f, 0.0f);	
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

	static CStopWatch	rotTimer;
	float yRot = rotTimer.GetElapsedSeconds() * 60.0f;

	modelViewMatrix.push();	
	M3DMatrix44f mCamera;
	cameraFrame.GetCameraMatrix(mCamera);
	modelViewMatrix.MultMatrix(mCamera);

	// draw the world upside down
	modelViewMatrix.push();
	modelViewMatrix.Scale(1.0f, -1.0f, 1.0f); // Flips the Y Axis
	modelViewMatrix.Translate(0.0f, 0.8f, 0.0f); // Scootch the world down a bit...
	glFrontFace(GL_CW);
	_DrawSongAndDance(yRot);
	glFrontFace(GL_CCW);
	modelViewMatrix.pop();

	// draw the solid ground
	glEnable(GL_BLEND);
	glBindTexture(GL_TEXTURE_2D, uiTextures[0]);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	static GLfloat vFloorColor[] = { 1.0f, 1.0f, 1.0f, 0.75f};
	shaderManager.useStockShader(GLT_SHADER_TEXTURE_MODULATE, 
								transformPipeline.GetMVPMatrix(), 
								vFloorColor, 
								0);
	floorBatch.draw();
	glDisable(GL_BLEND);

	_DrawSongAndDance(yRot);

	modelViewMatrix.pop();

	// Do the buffer Swap
	glutSwapBuffers();

	// Do it again
	glutPostRedisplay();
}

/**
 * Cleanup... such as deleting texture objects
 */
void OnExit()
{
	glDeleteTextures(3, uiTextures);
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

	gltShowVersionInfo();

	SetupRC();

	glutMainLoop();

	OnExit();

	return 0;
}
