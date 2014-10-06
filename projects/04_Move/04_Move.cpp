#include <GLTools.h>
#include <GL/glut.h>
#include "math3d.h"
#include <GLShaderManager.h>

GLBatch squareBatch;
GLShaderManager	shaderManager;

GLfloat blockSize = 0.1f;
GLfloat vVerts[] = {
	-blockSize, -blockSize, 0.0f, 
	blockSize,	-blockSize, 0.0f,
	blockSize,  blockSize,	0.0f,
	-blockSize, blockSize,	0.0f
};

GLfloat xPos = 0.0f;
GLfloat yPos = 0.0f;
GLfloat yRot = 0.0f;

// Window has changed size, or has just been created. In either case, we need
// to use the window dimensions to set the viewport and the projection matrix.
void ChangeSize(int w, int h)
{
	glViewport(0, 0, w, h);
}

// This function does any needed initialization on the rendering context. 
// This is the first opportunity to do any OpenGL related tasks.
void SetupRC()
{
	// Blue background
	glClearColor(0.0f, 0.0f, 1.0f, 1.0f );

	shaderManager.InitializeStockShaders();

	// Load up a triangle
	squareBatch.Begin(GL_TRIANGLE_FAN, 4);
	squareBatch.CopyVertexData3f(vVerts);
	squareBatch.End();
}

// Respond to arrow keys by moving the camera frame of reference
void SpecialKeys(int key, int x, int y)
{
	GLfloat stepSize = 0.025f;
	if(key == GLUT_KEY_UP)
		yPos += stepSize;

	if(key == GLUT_KEY_DOWN)
		yPos -= stepSize;

	stepSize = 5.0f;
	if(key == GLUT_KEY_LEFT)
		yRot -= stepSize;

	if(key == GLUT_KEY_RIGHT)
		yRot += stepSize;

	// Collision detection
	if(xPos < (-1.0f + blockSize)) xPos = -1.0f + blockSize;

	if(xPos > (1.0f - blockSize)) xPos = 1.0f - blockSize;

	if(yPos < (-1.0f + blockSize))  yPos = -1.0f + blockSize;

	if(yPos > (1.0f - blockSize)) yPos = 1.0f - blockSize;

	if ( key == GLUT_KEY_RIGHT ) {
	}

	glutPostRedisplay();
}

// Reset flags as appropriate in response to menu selections
void ProcessMenu(int value)
{
}

// Called to draw scene
void RenderScene(void)
{
	// Clear the window with current clearing color
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	GLfloat vRed[] = { 1.0f, 0.0f, 0.0f, 1.0f };

	M3DMatrix44f mFinalTransform;
	M3DMatrix44f mTranslationMatrix;
	M3DMatrix44f mRotationMatrix;

	// Just Translate
	m3dTranslationMatrix44(mTranslationMatrix, 0.0f, yPos, 0.0f);

	// Rotate 5 degrees everytime we redraw
	m3dRotationMatrix44(mRotationMatrix, m3dDegToRad(yRot), 0.0f, 1.0f, 0.0f);

	// Calculate the result
	// 这里位置顺序可以任意
	//m3dMatrixMultiply44(mFinalTransform, mTranslationMatrix, mRotationMatrix);
	m3dMatrixMultiply44(mFinalTransform, mRotationMatrix, mTranslationMatrix);

	// Do render
	shaderManager.UseStockShader(GLT_SHADER_FLAT, mFinalTransform, vRed);
	squareBatch.Draw();

	// Perform the buffer swap to display back buffer
	glutSwapBuffers();
}

// Main entry point for GLUT based programs
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

	GLenum err = glewInit();
	if (GLEW_OK != err) {
		fprintf(stderr, "GLEW Error: %s\n", glewGetErrorString(err));
		return 1;
	}

	SetupRC();

	glutMainLoop();
	return 0;
}
