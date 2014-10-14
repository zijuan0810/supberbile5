#include <GLTools.h>
#include <GL/glut.h>
#include <GLShaderManager.h>

#include "GLFrame.h"
#include "GLFrustum.h"
#include "GLBatch.h"
#include "GLMatrixStack.h"
#include "GLGeometryTransform.h"

// An assortment of needed classes
GLShaderManager		shaderManager;
GLMatrixStack		modelViewMatrix;
GLMatrixStack		projectionMatrix;
GLFrame				cameraFrame;
GLFrame             objectFrame;
GLFrustum			viewFrustum;

GLBatch				pointBatch;
GLBatch				lineBatch;
GLBatch				lineStripBatch;
GLBatch				lineLoopBatch;
GLBatch				triangleBatch;
GLBatch             triangleStripBatch;
GLBatch             triangleFanBatch;

GLGeometryTransform	transformPipeline;
M3DMatrix44f		shadowMatrix;

// color
GLfloat vGreen[] = {0.0f, 1.0f, 0.0f, 1.0f};
GLfloat vBlack[] = {0.0f, 0.0f, 0.0f, 1.0f};

// Keep track of effects step
static int nStep = 0;

// Window has changed size, or has just been created. In either case, we need
// to use the window dimensions to set the viewport and the projection matrix.
void ChangeSize(int w, int h)
{
	glViewport(0, 0, w, h);

	viewFrustum.SetPerspective(35.0f, float(w)/(float)h, 1.0f, 500.0f);
	projectionMatrix.setMatrix(viewFrustum.GetProjectionMatrix());
	modelViewMatrix.identity();
}

// This function does any needed initialization on the rendering context. 
// This is the first opportunity to do any OpenGL related tasks.
void SetupRC()
{
	// Black background
	glClearColor(0.7f, 0.7f, 0.7f, 1.0f );

	shaderManager.init();

	glEnable(GL_DEPTH_TEST);

	transformPipeline.SetMatrixStacks(modelViewMatrix, projectionMatrix);

	cameraFrame.MoveForward(-15.0f);

	// Some points, more or less in the shape of Florida
	GLfloat vCoast[24][3] = {
		{2.80, 1.20, 0.0 }, {2.0,  1.20, 0.0 },
		{2.0,  1.08, 0.0 },  {2.0,  1.08, 0.0 },
		{0.0,  0.80, 0.0 },  {-.32, 0.40, 0.0 },
		{-.48, 0.2, 0.0 },   {-.40, 0.0, 0.0 },
		{-.60, -.40, 0.0 },  {-.80, -.80, 0.0 },
		{-.80, -1.4, 0.0 },  {-.40, -1.60, 0.0 },
		{0.0, -1.20, 0.0 },  { .2, -.80, 0.0 },
		{.48, -.40, 0.0 },   {.52, -.20, 0.0 },
		{.48,  .20, 0.0 },   {.80,  .40, 0.0 },
		{1.20, .80, 0.0 },   {1.60, .60, 0.0 },
		{2.0, .60, 0.0 },    {2.2, .80, 0.0 },
		{2.40, 1.0, 0.0 },   {2.80, 1.0, 0.0 }
	};

	// Load point batch
	pointBatch.Begin(GL_POINTS, 24);
	pointBatch.CopyVertexData3f(vCoast);
	pointBatch.End();

	// Load as a bunch of line segments
	lineBatch.Begin(GL_LINES, 24);
	lineBatch.CopyVertexData3f(vCoast);
	lineBatch.End();

	// Load as a single line segment
	lineStripBatch.Begin(GL_LINE_STRIP, 24);
	lineStripBatch.CopyVertexData3f(vCoast);
	lineStripBatch.End();

	// Single line, connect first and last points
	lineLoopBatch.Begin(GL_LINE_LOOP, 24);
	lineLoopBatch.CopyVertexData3f(vCoast);
	lineLoopBatch.End();

	// For Triangles, we'll make a Pyramid
	GLfloat vPyramid[12][3] = { 
		{-2.0f, 0.0f, -2.0f}, 
		{2.0f, 0.0f, -2.0f}, 
		{0.0f, 4.0f, 0.0f},

		{2.0f, 0.0f, -2.0f},
		{2.0f, 0.0f, 2.0f},
		{0.0f, 4.0f, 0.0f},

		{2.0f, 0.0f, 2.0f},
		{-2.0f, 0.0f, 2.0f},
		{0.0f, 4.0f, 0.0f},

		{-2.0f, 0.0f, 2.0f},
		{-2.0f, 0.0f, -2.0f},
		{0.0f, 4.0f, 0.0f}
	};

	// Load triangle batch
	triangleBatch.Begin(GL_TRIANGLES, 12);
	triangleBatch.CopyVertexData3f(vPyramid);
	triangleBatch.End();

	// --- Begin fan
	// For a Triangle fan, just a 6 sided hex. Raise the center up a bit
	GLfloat vPoints[100][3];    // Scratch array, more than we need
	int nVerts = 0;
	GLfloat r = 3.0f;

	for (int i=0; i<3; ++i) {
		vPoints[nVerts][i] = 0.0f;
	}

	for (GLfloat angle=0.0f; angle < M3D_2PI; angle+= M3D_2PI/6.0f, nVerts++) {
		vPoints[nVerts][0] = static_cast<float>(cos(angle)) * r;
		vPoints[nVerts][1] = static_cast<float>(sin(angle)) * r;
		vPoints[nVerts][2] = -0.5f;
	}

	// Close the fan
	nVerts++;
	vPoints[nVerts][0] = r;
	vPoints[nVerts][1] = 0.0f;
	vPoints[nVerts][2] = 0.0f;

	// Load it up
	triangleFanBatch.Begin(GL_TRIANGLE_FAN, nVerts);
	triangleFanBatch.CopyVertexData3f(vPoints);
	triangleFanBatch.End();

	// --- End the fan


	// For triangle strips, a little ring or cylinder segment
	int iCounter = 0;
	GLfloat fRadius = 3.0f;
	for (GLfloat angle=0.0f; angle<=(2.0f*M3D_PI); angle+=0.3f) {
		GLfloat x = fRadius * sin(angle);
		GLfloat y = fRadius * cos(angle);

		// Specify the point and move the Z value up a little
		vPoints[iCounter][0] = x;
		vPoints[iCounter][1] = y;
		vPoints[iCounter][2] = -0.5f;
		iCounter++;

		vPoints[iCounter][0] = x;
		vPoints[iCounter][1] = y;
		vPoints[iCounter][2] = 0.5f;
		iCounter++;
	}
	// Close up the loop
	vPoints[iCounter][0] = vPoints[0][0];
	vPoints[iCounter][1] = vPoints[0][1];
	vPoints[iCounter][2] = -0.5;
	iCounter++;

	vPoints[iCounter][0] = vPoints[1][0];
	vPoints[iCounter][1] = vPoints[1][1];
	vPoints[iCounter][2] = 0.5;
	iCounter++;            

	// Load the triangle strip
	triangleStripBatch.Begin(GL_TRIANGLE_STRIP, iCounter);
	triangleStripBatch.CopyVertexData3f(vPoints);
	triangleStripBatch.End();  
}

void DrawWireFramedBatch(GLBatch* pBatch)
{
	// draw the batch solid green
	shaderManager.useStockShader(GLT_SHADER_FLAT, transformPipeline.GetMVPMatrix(), vGreen);
	pBatch->draw();

	// draw black outline
	glPolygonOffset(-1.0f, -1.0f);		// 设置偏移参数
	glEnable(GL_POLYGON_OFFSET_LINE);	// 设置多边形偏移模式

	// draw lines antialiased
	glEnable(GL_LINE_SMOOTH);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// draw black wireframe version of geometry
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glLineWidth(2.5f);
	shaderManager.useStockShader(GLT_SHADER_FLAT, transformPipeline.GetMVPMatrix(), vBlack);
	pBatch->draw();

	// Put everything back the way we found it
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

	shaderManager.useStockShader(GLT_SHADER_FLAT, transformPipeline.GetMVPMatrix(), vBlack);

	switch (nStep) {
	case 0:
		glPointSize(4.0f);
		pointBatch.draw();
		glPointSize(1.0f);
		break;
	case 1:
		glLineWidth(2.0f);
		lineBatch.draw();
		glLineWidth(1.0f);
		break;
	case 2:
		glLineWidth(2.0f);
		lineLoopBatch.draw();
		glLineWidth(1.0f);
		break;
	case 3:
		glLineWidth(2.0f);
		lineLoopBatch.draw();
		glLineWidth(1.0f);
		break;
	case 4:
		DrawWireFramedBatch(&triangleBatch);
		break;
	case 5:
		DrawWireFramedBatch(&triangleStripBatch);
		break;
	case 6:
		DrawWireFramedBatch(&triangleFanBatch);
		break;
	default:
		break;
	}

	modelViewMatrix.pop();

	// Perform the buffer swap to display back buffer
	glutSwapBuffers();
}

// Respond to arrow keys by moving the camera frame of reference
void SpecialKeys(int key, int x, int y)
{
	if(key == GLUT_KEY_UP) {
		objectFrame.RotateWorld(m3dDegToRad(-5.0f), 1.0f, 0.0f, 0.0f);
	}
	else if(key == GLUT_KEY_DOWN) {
		objectFrame.RotateWorld(m3dDegToRad(5.0f), 1.0f, 0.0f, 0.0f);
	}
	else if(key == GLUT_KEY_LEFT) {
		objectFrame.RotateWorld(m3dDegToRad(-5.0f), 0.0f, 1.0f, 0.0f);
	}
	else if(key == GLUT_KEY_RIGHT) {
		objectFrame.RotateWorld(m3dDegToRad(5.0f), 0.0f, 1.0f, 0.0f);
	}

	glutPostRedisplay();
}

// A normal ASCII key has been pressed.
// In this case, advance the scene when the space bar is pressed
void KeyPressFunc(unsigned char key, int x, int y)
{
	if(key == 32) {
		nStep++;
		if(nStep > 6) {
			nStep = 0;
		}
	}

	switch(nStep) {
	case 0: 
		glutSetWindowTitle("GL_POINTS");
		break;
	case 1:
		glutSetWindowTitle("GL_LINES");
		break;
	case 2:
		glutSetWindowTitle("GL_LINE_STRIP");
		break;
	case 3:
		glutSetWindowTitle("GL_LINE_LOOP");
		break;
	case 4:
		glutSetWindowTitle("GL_TRIANGLES");
		break;
	case 5:
		glutSetWindowTitle("GL_TRIANGLE_STRIP");
		break;
	case 6:
		glutSetWindowTitle("GL_TRIANGLE_FAN");
		break;
	}

	glutPostRedisplay();
}

// Main entry point for GLUT based programs
int main(int argc, char* argv[])
{
	gltSetWorkingDirectory(argv[0]);

	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH | GLUT_STENCIL);
	glutInitWindowSize(800, 600);
	glutCreateWindow("Triangle");
	glutReshapeFunc(ChangeSize);
	glutDisplayFunc(RenderScene);

	glutKeyboardFunc(KeyPressFunc);
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
