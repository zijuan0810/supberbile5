#include <GLTools.h>
#include <GL/glut.h>
#include <GLShaderManager.h>

#include "GLFrame.h"
#include "GLFrustum.h"
#include "GLBatch.h"
#include "GLMatrixStack.h"
#include "GLGeometryTransform.h"

#include <math.h>

// An assortment of needed classes
GLShaderManager		shaderManager;
GLMatrixStack		modelViewMatrix;
GLMatrixStack		projectionMatrix;
GLFrame				cameraFrame;
GLFrame             objectFrame;
GLFrustum			viewFrustum;

GLTriangleBatch     sphereBatch;
GLTriangleBatch     torusBatch;
GLTriangleBatch     cylinderBatch;
GLTriangleBatch     coneBatch;
GLTriangleBatch     diskBatch;

GLGeometryTransform	transformPipeline;
M3DMatrix44f		shadowMatrix;


GLfloat vGreen[] = { 0.0f, 1.0f, 0.0f, 1.0f };
GLfloat vBlack[] = { 0.0f, 0.0f, 0.0f, 1.0f };


// Keep track of effects step
int nStep = 0;

// Window has changed size, or has just been created. In either case, we need
// to use the window dimensions to set the viewport and the projection matrix.
void ChangeSize(int w, int h)
{
	glViewport(0, 0, w, h);

	viewFrustum.setPerspective(35.0f, float(w) / float(h), 1.0f, 500.0f);
	projectionMatrix.setMatrix(viewFrustum.GetProjectionMatrix());

	modelViewMatrix.identity();
}


// This function does any needed initialization on the rendering context. 
// This is the first opportunity to do any OpenGL related tasks.
void SetupRC()
{
	// Black background
	glClearColor(0.7f, 0.7f, 0.7f, 1.0f);

	shaderManager.init();

	glEnable(GL_DEPTH_TEST);

	transformPipeline.setMatrixStacks(modelViewMatrix, projectionMatrix);

	cameraFrame.MoveForward(-15.0f);

	// Sphere
	gltCreateSphere(sphereBatch, 3.0f, 10, 20);
	// Torus
	gltCreateTorus(torusBatch, 3.0f, 0.75f, 15, 15);
	// Cylinder
	gltMakeCylinder(cylinderBatch, 2.0f, 2.0f, 3.0f, 13, 2);
	// Cone
	gltMakeCylinder(coneBatch, 2.0f, 0.0f, 3.0f, 13, 2);
	// Disk
	gltMakeDisk(diskBatch, 1.5f, 3.0f, 13, 3);
}

// Respond to arrow keys by moving the camera frame of reference
void SpecialKeys(int key, int x, int y)
{
	if(key == GLUT_KEY_UP)
		objectFrame.RotateWorld(m3dDegToRad(-5.0f), 1.0f, 0.0f, 0.0f);

	if(key == GLUT_KEY_DOWN)
		objectFrame.RotateWorld(m3dDegToRad(5.0f), 1.0f, 0.0f, 0.0f);

	if(key == GLUT_KEY_LEFT)
		objectFrame.RotateWorld(m3dDegToRad(-5.0f), 0.0f, 1.0f, 0.0f);

	if(key == GLUT_KEY_RIGHT)
		objectFrame.RotateWorld(m3dDegToRad(5.0f), 0.0f, 1.0f, 0.0f);

	glutPostRedisplay();
}

// A normal ASCII key has been pressed.
// In this case, advance the scene when the space bar is pressed
void KeyPressFunc(unsigned char key, int x, int y)
{
	if(key == 32) {
		nStep++;
		if(nStep > 4) {
			nStep = 0;
		}
	}

	switch(nStep) {
	case 0: 
		glutSetWindowTitle("Sphere");
		break;
	case 1:
		glutSetWindowTitle("Torus");
		break;
	case 2:
		glutSetWindowTitle("Cylinder");
		break;
	case 3:
		glutSetWindowTitle("Cone");
		break;
	case 4:
		glutSetWindowTitle("Disk");
		break;
	}

	glutPostRedisplay();
}

// Reset flags as appropriate in response to menu selections
void ProcessMenu(int value)
{
}

void _drawWireFramedBatch(GLTriangleBatch* pBatch)
{
	shaderManager.useStockShader(GLT_SHADER_FLAT, transformPipeline.GetMVPMatrix(),
		vGreen);
	pBatch->draw();

	// draw black outline
	glPolygonOffset(-1.0f, -1.0f);
	glEnable(GL_LINE_SMOOTH);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_POLYGON_OFFSET_LINE);
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glLineWidth(2.5f);
	shaderManager.useStockShader(GLT_SHADER_FLAT, transformPipeline.GetMVPMatrix(),
		vBlack);
	pBatch->draw();

	// Restore polygon mode and depth testing
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glDisable(GL_POLYGON_OFFSET_LINE);
	glLineWidth(1.0f);
	glDisable(GL_BLEND);
	glDisable(GL_LINE_SMOOTH);
}

// Called to draw scene
void RenderScene(void)
{
	// Clear the window with current clearing color
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	modelViewMatrix.push();
	M3DMatrix44f mCamera;
	cameraFrame.GetCameraMatrix(mCamera);
	modelViewMatrix.MultMatrix(mCamera);

	M3DMatrix44f mObjectFrame;
	objectFrame.GetMatrix(mObjectFrame);
	modelViewMatrix.MultMatrix(mObjectFrame);

	shaderManager.useStockShader(GLT_SHADER_FLAT, transformPipeline.GetMVPMatrix(), 
		vBlack);

	switch(nStep) {
	case 0:
		_drawWireFramedBatch(&sphereBatch);
		break;
	case 1:
		_drawWireFramedBatch(&torusBatch);
		break;
	case 2:
		_drawWireFramedBatch(&cylinderBatch);
		break;
	case 3:
		_drawWireFramedBatch(&coneBatch);
		break;
	case 4:
		_drawWireFramedBatch(&diskBatch);
		break;
	}

	modelViewMatrix.pop();

	// Flush drawing commands
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
