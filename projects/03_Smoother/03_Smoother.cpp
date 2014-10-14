#include <GLTools.h>
#include <GL/glut.h>
#include <GLShaderManager.h>

#include "GLFrustum.h"

GLShaderManager	shaderManager;

GLFrustum viewFrustum;
GLBatch smallStarBatch;
GLBatch mediumStarBatch;
GLBatch largeStarBatch;
GLBatch mountainRangeBatch;
GLBatch moonBatch;

// Array of small stars
#define SMALL_STARS     100
#define MEDIUM_STARS     40
#define LARGE_STARS      15

#define SCREEN_X        800
#define SCREEN_Y        600

// Window has changed size, or has just been created. In either case, we need
// to use the window dimensions to set the viewport and the projection matrix.
void ChangeSize(int w, int h)
{
	glViewport(0, 0, w, h);

	 // Establish clipping volume (left, right, bottom, top, near, far)
	viewFrustum.SetOrthographic(0.0f, SCREEN_X, 0.0f, SCREEN_Y, -1.0f, 1.0f);
}

// This function does any needed initialization on the rendering context. 
// This is the first opportunity to do any OpenGL related tasks.
void SetupRC()
{
	shaderManager.init();

	M3DVector3f vVerts[SMALL_STARS];       // SMALL_STARS is the largest batch we are going to need

	// Populate star list
	smallStarBatch.begin(GL_POINTS, SMALL_STARS);
	for ( int i = 0; i < SMALL_STARS; i++ ) {
		vVerts[i][0] = (GLfloat)(rand() % SCREEN_X);
		vVerts[i][1] = (GLfloat)(rand() % (SCREEN_Y - 100)) + 100.0f;
		vVerts[i][2] = 0.0f;
	}
	smallStarBatch.CopyVertexData3f(vVerts);
	smallStarBatch.end();

	// Populate star list
	mediumStarBatch.begin(GL_POINTS, MEDIUM_STARS);
	for ( int i = 0; i < MEDIUM_STARS; i++ ) {
		vVerts[i][0] = (GLfloat)(rand() % SCREEN_X);
		vVerts[i][1] = (GLfloat)(rand() % (SCREEN_Y - 100)) + 100.0f;
		vVerts[i][2] = 0.0f; 
	}
	mediumStarBatch.CopyVertexData3f(vVerts);
	mediumStarBatch.end();

	// Populate star list
	largeStarBatch.begin(GL_POINTS, LARGE_STARS);
	for ( int i = 0; i < LARGE_STARS; i++ ) {
		vVerts[i][0] = (GLfloat)(rand() % SCREEN_X);
		vVerts[i][1] = (GLfloat)(rand() % (SCREEN_Y - 100)) + 100.0f;
		vVerts[i][2] = 0.0f;
	}
	largeStarBatch.CopyVertexData3f(vVerts);
	largeStarBatch.end();

	M3DVector3f vMountains[12] = { 
		0.0f, 25.0f, 0.0f, 
		50.0f, 100.0f, 0.0f,
		100.0f, 25.0f, 0.0f,
		225.0f, 125.0f, 0.0f,
		300.0f, 50.0f, 0.0f,
		375.0f, 100.0f, 0.0f,
		460.0f, 25.0f, 0.0f,
		525.0f, 100.0f, 0.0f,
		600.0f, 20.0f, 0.0f,
		675.0f, 70.0f, 0.0f,
		750.0f, 25.0f, 0.0f,
		800.0f, 90.0f, 0.0f 
	};    
	mountainRangeBatch.begin(GL_LINE_STRIP, 12);
	mountainRangeBatch.CopyVertexData3f(vMountains);
	mountainRangeBatch.end();

	// The Moon
	GLfloat x = 700.0f;     // Location and radius of moon
	GLfloat y = 500.0f;
	GLfloat r = 50.0f;
	GLfloat angle = 0.0f;   // Another looping variable

	int nVerts = 0;
	vVerts[nVerts][0] = x;
	vVerts[nVerts][1] = y;
	vVerts[nVerts][2] = 0.0f;
	for ( angle = 0; angle < 2.0f * 3.141592f; angle += 0.2f ) {
		nVerts++;
		vVerts[nVerts][0] = x + float(cos(angle)) * r;
		vVerts[nVerts][1] = y + float(sin(angle)) * r;
		vVerts[nVerts][2] = 0.0f;
	}
	nVerts++;

	vVerts[nVerts][0] = x + r;;
	vVerts[nVerts][1] = y;
	vVerts[nVerts][2] = 0.0f;
	moonBatch.begin(GL_TRIANGLE_FAN, 34);
	moonBatch.CopyVertexData3f(vVerts);
	moonBatch.end();     

	// Black background
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f );

	// test
	M3DMatrix33f m;
	m3dRotationMatrix33(m, m3dDegToRad(45.0), 1.0, 0.0, 0.0);
	int a = 10;
}

// Respond to arrow keys by moving the camera frame of reference
void SpecialKeys(int key, int x, int y)
{
}

// Reset flags as appropriate in response to menu selections
void ProcessMenu(int value)
{
	switch ( value ) {
	case 1: 
		{
			// Turn on  antialiasing, and give hint to do the best job possible
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

			glEnable(GL_BLEND);

			glEnable(GL_POINT_SMOOTH);
			glHint(GL_POINT_SMOOTH_HINT, GL_NICEST);

			glEnable(GL_LINE_SMOOTH);
			glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);

			glEnable(GL_POINT_SMOOTH);
			glHint(GL_POINT_SMOOTH_HINT, GL_NICEST);
		}
		break;

	case 2:
		{
			glDisable(GL_BLEND);
			glDisable(GL_LINE_SMOOTH);
			glDisable(GL_POINT_SMOOTH);
			glDisable(GL_POLYGON_SMOOTH);
		}
		break;

	default:
		break;
	}

	// Trigger a redraw
	glutPostRedisplay();
}

// Called to draw scene
void RenderScene(void)
{
	// Clear the window with current clearing color
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	// Everything is white
	GLfloat vWhite[] = {1.0f, 1.0f, 1.0f, 1.0f};
	GLfloat vRed[] = {1.0f, 0.0f, 0.0f, 1.0f};

	// draw the "moon"
	glEnable(GL_MULTISAMPLE);	// 开启多重采样
	shaderManager.useStockShader(GLT_SHADER_FLAT, viewFrustum.GetProjectionMatrix(), vRed);
	moonBatch.draw();
	glDisable(GL_MULTISAMPLE);

	shaderManager.useStockShader(GLT_SHADER_FLAT, viewFrustum.GetProjectionMatrix(), vWhite);

	// draw samll stars
	glPointSize(1.0f);
	smallStarBatch.draw();

	// draw medium sized stars
	glPointSize(4.0f);
	mediumStarBatch.draw();

	// draw largest stars
	glPointSize(8.0f);
	largeStarBatch.draw();

	// draw distant horizon
	glLineWidth(3.5f);
	mountainRangeBatch.draw();

	// Perform the buffer swap to display back buffer
	glutSwapBuffers();
}

// Main entry point for GLUT based programs
int main(int argc, char* argv[])
{
	gltSetWorkingDirectory(argv[0]);

	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH | GLUT_STENCIL | GLUT_MULTISAMPLE);
	glutInitWindowSize(800, 600);
	glutCreateWindow("Triangle");

	// Create the Menu
	glutCreateMenu(ProcessMenu);
	glutAddMenuEntry("Antialiased Rendering",1);
	glutAddMenuEntry("Normal Rendering",2);
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
