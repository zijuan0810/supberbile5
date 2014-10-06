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

GLFrame	cameraFrame;

#define NUM_SPHERES 50
GLFrame spheres[NUM_SPHERES];


/**
 * Window has changed size, or has just been created. In either case, we need to use the window 
 * dimensions to set the viewport and the projection matrix.
 */
void ChangeSize(int w, int h)
{
	glViewport(0, 0, w, h);

	// Create the projection matrix, and load it on the projection matrix stack
	viewFrustum.SetPerspective(35.0f, (float)w/(float)h, 1.0f, 1000.0f);
	projectionMatrix.LoadMatrix(viewFrustum.GetProjectionMatrix());

	// Set the transform pipeline to use the two matrix stack
	tranformPipeline.SetMatrixStacks(modelViewMatrix, projectionMatrix);
}

/**
 * This function does any needed initialization on the rendering context.
 * This is the first opportunity to do any OpenGL related tasks.
 */
void SetupRC()
{
	// Blue background
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f );

	shaderManager.InitializeStockShaders();

	glEnable(GL_DEPTH_TEST);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	// This make torus
	gltMakeTorus(torusBatch, 0.4f, 0.15f, 30, 30);
	// This make sphere
	gltMakeSphere(sphereBatch, 0.2f, 30, 30);


	// This make a floor
	floorBatch.Begin(GL_LINES, 324);
	for (GLfloat x=-20.0f; x<=20.0f; x+=0.5f) {
		floorBatch.Vertex3f(x, -0.55f, 20.0f);
		floorBatch.Vertex3f(x, -0.55f, -20.0f);

		floorBatch.Vertex3f(20.0f, -0.55f, x);
		floorBatch.Vertex3f(-20.0f, -0.55f, x);
	}
	floorBatch.End();

	// 这里我们并不需要创建50个实际的球体，只要将同一个球体绘制50次就可以
	// 随机放置sphere
	for (int i=0; i<NUM_SPHERES; ++i) {
		GLfloat x = (GLfloat)((rand() % 400) - 200) * 0.1f;
		GLfloat z = (GLfloat)((rand() % 400) - 200) * 0.1f;
		spheres[i].SetOrigin(x, 0.0f, z);
	}
}

/**
 * Respond to arrow keys by moving the camera frame of reference
 */
void SpecialKeys(int key, int x, int y)
{
	float linear = 0.1f;
	float angular = (float)m3dDegToRad(5.0f);

	if ( key == GLUT_KEY_UP ) {
		cameraFrame.MoveUp(linear);
	}
	else if ( key == GLUT_KEY_DOWN ) {
		cameraFrame.MoveUp(-linear);
	}
	else if ( key == GLUT_KEY_LEFT ) {
		cameraFrame.RotateWorld(angular, 0.0f, 1.0f, 0.0);
	}
	else if ( key == GLUT_KEY_RIGHT ) {
		cameraFrame.RotateWorld(-angular, 0.0f, 1.0f, 0.0);
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
	float yRot = rotTimer.GetElapsedSeconds() * 60.0f;

	// Save the current modelview matrix (the identity matrix)
	modelViewMatrix.PushMatrix();

	M3DMatrix44f mCamera;
	cameraFrame.GetCameraMatrix(mCamera);
	modelViewMatrix.PushMatrix(mCamera);

	// Transform the light position into eye coordinates
	M3DVector4f vLightPos = {0.0f, 10.0f, 5.0f, 1.0f};
	M3DVector4f vLightEyePos;
	m3dTransformVector4(vLightEyePos, vLightPos, mCamera);

	// Draw the ground
	shaderManager.UseStockShader(GLT_SHADER_FLAT, tranformPipeline.GetModelViewProjectionMatrix(),
		vFloorColor);
	floorBatch.Draw();

	// Draw the spinning Torus
	modelViewMatrix.Translate(0.0f, 0.0f, -2.5f);
	// 保存平移
	modelViewMatrix.PushMatrix();

	// 应用旋转并绘制圆环
	modelViewMatrix.Rotate(yRot, 0.0f, 1.0f, 0.0f);
	//shaderManager.UseStockShader(GLT_SHADER_FLAT, tranformPipeline.GetModelViewProjectionMatrix(), 
	//	vTorusColor);
	shaderManager.UseStockShader(GLT_SHADER_POINT_LIGHT_DIFF,
								tranformPipeline.GetModelViewMatrix(),
								tranformPipeline.GetProjectionMatrix(),
								vLightEyePos,
								vTorusColor);
	torusBatch.Draw();

	// Restore the previous modelview matrix (the identity matrix)
	modelViewMatrix.PopMatrix();

	// 应用另一个旋转，然后平移，再绘制球体
	//modelViewMatrix.PushMatrix();
	modelViewMatrix.Rotate(-2.0f*yRot, 0.0f, 1.0f, 0.0f);
	modelViewMatrix.Translate(0.8f, 0.0f, 0.0f);
	//shaderManager.UseStockShader(GLT_SHADER_FLAT, tranformPipeline.GetModelViewProjectionMatrix(),
	//							vSphereColor);

	shaderManager.UseStockShader(GLT_SHADER_POINT_LIGHT_DIFF,
								tranformPipeline.GetModelViewMatrix(),
								tranformPipeline.GetProjectionMatrix(),
								vLightEyePos,
								vSphereColor);
	sphereBatch.Draw();

	modelViewMatrix.PopMatrix();
	for (int i=0; i<NUM_SPHERES; ++i) {
		modelViewMatrix.PushMatrix();
		modelViewMatrix.MultMatrix(spheres[i]);
		//shaderManager.UseStockShader(GLT_SHADER_FLAT, 
		//							tranformPipeline.GetModelViewProjectionMatrix(),
		//							vSphereColor);
		shaderManager.UseStockShader(GLT_SHADER_POINT_LIGHT_DIFF,
									tranformPipeline.GetModelViewMatrix(),
									tranformPipeline.GetProjectionMatrix(),
									vLightEyePos,
									vSphereColor);
		sphereBatch.Draw();
		modelViewMatrix.PopMatrix();
	}

	modelViewMatrix.PopMatrix();

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
