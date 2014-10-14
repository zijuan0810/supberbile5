#include <GLTools.h>
#include <GL/glut.h>
#include <GLShaderManager.h>

#include "GLFrame.h"
#include "GLFrustum.h"
#include "GLMatrixStack.h"
#include "GLGeometryTransform.h"

GLShaderManager	shaderManager;

GLFrame				viewFrame;
GLFrustum			viewFrustum;
GLTriangleBatch		torusBatch;
GLMatrixStack		modelViewMatrix;
GLMatrixStack		projectionMatrix;
GLGeometryTransform	transformPiepline;

// Flags for effects
bool bCulling = false;
bool bDepthTest = false;

// Window has changed size, or has just been created. In either case, we need
// to use the window dimensions to set the viewport and the projection matrix.
void ChangeSize(int w, int h)
{
	// Prevent a divide by zero
	if ( h == 0 ) {
		h = 1;
	}

	glViewport(0, 0, w, h);

	viewFrustum.SetPerspective(35.0f, float(w)/float(h), 1.0f, 100.0f);

	projectionMatrix.setMatrix(viewFrustum.GetProjectionMatrix());
	transformPiepline.SetMatrixStacks(modelViewMatrix, projectionMatrix);
}

// This function does any needed initialization on the rendering context. 
// This is the first opportunity to do any OpenGL related tasks.
void SetupRC()
{
	// Blue background
	glClearColor(0.3f, 0.3f, 0.3f, 1.0f );

	shaderManager.init();
	viewFrame.MoveForward(7.0f);

	// Make the torus
	gltMakeTorus(torusBatch, 1.0f, 0.3f, 52, 25);

	glPointSize(4.0f);
}

// Called to draw scene
void RenderScene(void)
{
	// Clear the window with current clearing color
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	// Turn culling on if flag is set
	if ( bCulling ) {
		glEnable(GL_CULL_FACE);
	}
	else {
		glDisable(GL_CULL_FACE);
	}

	// Enable depth testing if flag is set
	if ( bDepthTest ) {
		glEnable(GL_DEPTH_TEST);
	}
	else {
		glDisable(GL_DEPTH_TEST);
	}

	modelViewMatrix.push(viewFrame);

	GLfloat vRed[] = { 1.0f, 0.0f, 0.0f, 1.0f };
	shaderManager.useStockShader(GLT_SHADER_DEFAULT_LIGHT, transformPiepline.GetModelViewMatrix(),
		transformPiepline.GetProjectionMatrix(), vRed);

	torusBatch.draw();

	modelViewMatrix.pop();

	// Perform the buffer swap to display back buffer
	glutSwapBuffers();
}

// Reset flags as appropriate in response to menu selections
void ProcessMenu(int nValue)
{
	switch (nValue) {
	case 1: 
		{
			bDepthTest = !bDepthTest;
			if ( bDepthTest ) {
				glutSetWindowTitle("Depth Test On");
			}
			else {
				glutSetWindowTitle("Depth Test Off");
			}
		}
		break;
	case 2:
		{
			bCulling = !bCulling;
			if ( bCulling ) {
				glutSetWindowTitle("Cull Test On");
			}
			else {
				glutSetWindowTitle("Cull Test Off");
			}
		}
		break;
	case 3:
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		break;
	case 4:
		//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		//glPolygonMode(GL_BACK, GL_LINE);
		glPolygonMode(GL_FRONT, GL_LINE);
		break;
	case 5:
		glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
		break;
	default:
		break;
	}

	glutPostRedisplay();
}

void SpecialKeys(int key, int x, int y)
{
	switch (key) {
	case GLUT_KEY_UP:
		viewFrame.RotateWorld(m3dDegToRad(-5.0f), 1.0f, 0.0f, 0.0f);
		break;
	case GLUT_KEY_DOWN:
		viewFrame.RotateWorld(m3dDegToRad(5.0f), 1.0f, 0.0f, 0.0f);
		break;
	case GLUT_KEY_LEFT:
		viewFrame.RotateWorld(m3dDegToRad(-5.0f), 0.0f, 1.0f, 0.0f);
		break;
	case GLUT_KEY_RIGHT:
		viewFrame.RotateWorld(m3dDegToRad(5.0f), 0.0f, 1.0f, 0.0f);
		break;
	default:
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

	glutSpecialFunc(SpecialKeys);

	// Create the menu
	glutCreateMenu(ProcessMenu);
	glutAddMenuEntry("Toggle depth test", 1);
	glutAddMenuEntry("Toggle cull backface", 2);
	glutAddMenuEntry("Set Fill Mode", 3);
	glutAddMenuEntry("Set Line Mode", 4);
	glutAddMenuEntry("Set Point Mode", 5);
	glutAttachMenu(GLUT_RIGHT_BUTTON);

	GLenum err = glewInit();
	if (GLEW_OK != err) {
		fprintf(stderr, "GLEW Error: %s\n", glewGetErrorString(err));
		return 1;
	}

	SetupRC();

	glutMainLoop();
	return 0;
}
