#include <GLTools.h>
#include <GL/glut.h>
#include <GLShaderManager.h>

GLShaderManager	shaderManager;

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

	shaderManager.init();
}

// Called to draw scene
void RenderScene(void)
{
	// Clear the window with current clearing color
	glClearColor(0.0f, 0.0f, 1.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	// Now set scissor to smaller red sub region
	glClearColor(1.0f, 0.0f, 0.0f, 0.0f);
	glScissor(100, 100, 600, 400);
	glEnable(GL_SCISSOR_TEST);
	glClear(GL_COLOR_BUFFER_BIT);

	// Finally, an even smaller green rectangle
	glClearColor(0.0f, 1.0f, 0.0f, 0.0);
	glScissor(200, 200, 400, 200);
	glClear(GL_COLOR_BUFFER_BIT);

	// Turn scissor back off for next render
	glDisable(GL_SCISSOR_TEST);

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
	glutReshapeFunc(ChangeSize);
	glutDisplayFunc(RenderScene);

	GLenum err = glewInit();
	if (GLEW_OK != err) {
		fprintf(stderr, "GLEW Error: %s\n", glewGetErrorString(err));
		return 1;
	}

	SetupRC();

	glutMainLoop();
	return 0;
}
