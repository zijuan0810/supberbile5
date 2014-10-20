#include <GLTools.h>
#include <GL/glut.h>
#include <GLShaderManager.h>

GLShaderManager	shaderManager;

GLBatch squareBatch;
GLBatch greenBatch;
GLBatch redBatch;
GLBatch blueBatch;
GLBatch blackBatch;

GLfloat blockSize = 0.2f;
GLfloat vVerts[] = {
	-blockSize, -blockSize, 0.0f,
	 blockSize, -blockSize, 0.0f,
	 blockSize,  blockSize, 0.0f,
	-blockSize,  blockSize, 0.0f
};

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
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f );

	shaderManager.init();

	// Load up a triangle fan
	squareBatch.begin(GL_TRIANGLE_FAN, 4);
	squareBatch.CopyVertexData3f(vVerts);
	squareBatch.end();

	// Solid
	GLfloat vBlock1[] = {
		0.25f, 0.25f, 0.0f,
		0.75f, 0.25f, 0.0f,
		0.75f, 0.75f, 0.0f,
		0.25f, 0.75f, 0.0f
	};
	greenBatch.begin(GL_TRIANGLE_FAN, 4);
	greenBatch.CopyVertexData3f(vBlock1);
	greenBatch.end();

	GLfloat vBlock2[] = { 
		-0.75f, 0.25f, 0.0f,
		-0.25f, 0.25f, 0.0f,
		-0.25f, 0.75f, 0.0f,
		-0.75f, 0.75f, 0.0f
	};
	redBatch.begin(GL_TRIANGLE_FAN, 4);
	redBatch.CopyVertexData3f(vBlock2);
	redBatch.end();

	GLfloat vBlock3[] = { 
		-0.75f, -0.75f, 0.0f,
		-0.25f, -0.75f, 0.0f,
		-0.25f, -0.25f, 0.0f,
		-0.75f, -0.25f, 0.0f
	};
	blueBatch.begin(GL_TRIANGLE_FAN, 4);
	blueBatch.CopyVertexData3f(vBlock3);
	blueBatch.end();

	GLfloat vBlock4[] = { 
		0.25f, -0.75f, 0.0f,
		0.75f, -0.75f, 0.0f,
		0.75f, -0.25f, 0.0f,
		0.25f, -0.25f, 0.0f};
	blackBatch.begin(GL_TRIANGLE_FAN, 4);
	blackBatch.CopyVertexData3f(vBlock4);
	blackBatch.end();
}

// Respond to arrow keys by moving the camera frame of reference
void SpecialKeys(int key, int x, int y)
{
	GLfloat stepSize = 0.025f;

	GLfloat blockX = vVerts[0];		// Upper left X
	GLfloat blockY = vVerts[7];		// Upper right Y

	if ( key == GLUT_KEY_UP ) {
		blockY += stepSize;
	}
	else if ( key == GLUT_KEY_DOWN ) {
		blockY -= stepSize;
	}
	else if ( key == GLUT_KEY_LEFT ) {
		blockX -= stepSize;
	}
	else if ( key == GLUT_KEY_RIGHT ) {
		blockX += stepSize;
	}

	// Collision detection
	if ( blockX < -1.0f ) { blockX = -1.0f; }
	if(blockX > (1.0f - blockSize * 2)) blockX = 1.0f - blockSize * 2;;
	if(blockY < -1.0f + blockSize * 2)  blockY = -1.0f + blockSize * 2;
	if(blockY > 1.0f) blockY = 1.0f;

	// Recalculate vertex positions
	vVerts[0] = blockX;
	vVerts[1] = blockY - blockSize*2;

	vVerts[3] = blockX + blockSize*2;
	vVerts[4] = blockY - blockSize*2;

	vVerts[6] = blockX + blockSize*2;
	vVerts[7] = blockY;

	vVerts[9] = blockX;
	vVerts[10] = blockY;

	squareBatch.CopyVertexData3f(vVerts);

	glutPostRedisplay();
}

// Called to draw scene
void RenderScene(void)
{
	// Clear the window with current clearing color
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	GLfloat vRed[] = { 1.0f, 0.0f, 0.0f, 0.5f };
	GLfloat vGreen[] = { 0.0f, 1.0f, 0.0f, 1.0f };
	GLfloat vBlue[] = { 0.0f, 0.0f, 1.0f, 1.0f };
	GLfloat vBlack[] = { 0.0f, 0.0f, 0.0f, 1.0f };

	glEnable(GL_BLEND);
	glBlendFunc(GL_DST_ALPHA, GL_ZERO);

	shaderManager.useStockShader(GLT_SHADER_IDENTITY, vGreen);
	greenBatch.draw();

	shaderManager.useStockShader(GLT_SHADER_IDENTITY, vRed);
	redBatch.draw();

	shaderManager.useStockShader(GLT_SHADER_IDENTITY, vBlue);
	blueBatch.draw();

	shaderManager.useStockShader(GLT_SHADER_IDENTITY, vBlack);
	blackBatch.draw();

	glDisable(GL_BLEND);

	// begin blending
	glEnable(GL_BLEND);
	//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glBlendFunc(GL_ONE, GL_ZERO);
	shaderManager.useStockShader(GLT_SHADER_IDENTITY, vRed);
	squareBatch.draw();
	glDisable(GL_BLEND);

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
