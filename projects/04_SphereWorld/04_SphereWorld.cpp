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

GLShaderManager	shaderManager;

GLMatrixStack		modelViewMatrix;	// Modelview matrix
GLMatrixStack		projectionMatrix;	// Projection matrix
GLFrustum			viewFrustum;
GLGeometryTransform	tranformPipeline;

GLBatch				floorBatch;
GLTriangleBatch		torusBatch;
GLTriangleBatch		sphereBatch;


/**
 * Window has changed size, or has just been created. In either case, we need to use the window 
 * dimensions to set the viewport and the projection matrix.
 */
void ChangeSize(int w, int h)
{
	glViewport(0, 0, w, h);

	// Create the projection matrix, and load it on the projection matrix stack
	viewFrustum.setPerspective(35.0f, (float)w/(float)h, 1.0f, 1000.0f);
	projectionMatrix.setMatrix(viewFrustum.GetProjectionMatrix());

	// Set the transform pipeline to use the two matrix stack
	tranformPipeline.setMatrixStacks(modelViewMatrix, projectionMatrix);
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

	glEnable(GL_DEPTH_TEST);
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	// This make torus
	gltCreateTorus(torusBatch, 0.4f, 0.15f, 30, 30);
	// This make sphere
	gltCreateSphere(sphereBatch, 0.2f, 30, 30);


	// This make a floor
	floorBatch.begin(GL_LINES, 324);
	for (GLfloat x=-20.0f; x<=20.0f; x+=0.5f) {
		floorBatch.Vertex3f(x, -0.55f, 20.0f);
		floorBatch.Vertex3f(x, -0.55f, -20.0f);

		floorBatch.Vertex3f(20.0f, -0.55f, x);
		floorBatch.Vertex3f(-20.0f, -0.55f, x);
	}
	floorBatch.end();
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

	// Color values
	static GLfloat vFloorColor[] = { 0.0f, 1.0f, 0.0f, 1.0f};
	static GLfloat vTorusColor[] = { 1.0f, 0.0f, 0.0f, 1.0f };
	static GLfloat vSphereColor[] = { 0.0f, 0.0f, 1.0f, 1.0f };

	// Time Based animation
	static CStopWatch	rotTimer;
	float yRot = rotTimer.delta() * 60.0f;

	// Save the current modelview matrix (the identity matrix)
	modelViewMatrix.push();

	// draw the ground
	shaderManager.useStockShader(GLT_SHADER_FLAT, tranformPipeline.GetMVPMatrix(),
		vFloorColor);
	floorBatch.draw();

	// draw the spinning Torus
	modelViewMatrix.moveTo(0.0f, 0.0f, -2.5f);
	modelViewMatrix.rotateTo(yRot, 0.0f, 1.0f, 0.0f);
	shaderManager.useStockShader(GLT_SHADER_FLAT, tranformPipeline.GetMVPMatrix(), 
		vTorusColor);
	torusBatch.draw();

	// Restore the previous modelview matrix (the identity matrix)
	//modelViewMatrix.pop();

	// 应用另一个旋转，然后平移，再绘制球体
	modelViewMatrix.push();
	modelViewMatrix.rotateTo(-20.0f*yRot, 0.0f, 1.0f, 0.0f);
	modelViewMatrix.moveTo(0.8f, 0.0f, 0.0f);
	shaderManager.useStockShader(GLT_SHADER_FLAT, tranformPipeline.GetMVPMatrix(),
		vSphereColor);
	sphereBatch.draw();

	modelViewMatrix.pop();
	modelViewMatrix.pop();

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
