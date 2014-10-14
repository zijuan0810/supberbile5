#include "GLTool-ext.h"

#define NUM_SPHERES 50
GLFrame spheres[NUM_SPHERES];

GLShaderManager		shaderManager;		// Shader Manager
GLMatrixStack			modelViewMatrix;		// Modelview Matrix
GLMatrixStack			projectionMatrix;		// Projection Matrix
GLFrustum			viewFrustum;			// View Frustum
GLGeometryTransform	transformPipeline;		// Geometry Transform Pipeline
GLFrame				cameraFrame;			// Camera frame

GLTriangleBatch		torusBatch;
GLTriangleBatch		sphereBatch;
GLBatch				floorBatch;
GLBatch				logoBatch;

GLuint	uiTextures[4];
GLint	rectReplaceShader;
GLint	locRectMVP;
GLint	locRectTexture;

// called to draw dancing objects
static void DrawSongAndDance(GLfloat yRot)
{
	static GLfloat vWhite[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	static GLfloat vLightPos[] = { 0.0f, 3.0f, 0.0f, 1.0f };

	// get the light position in eye space
	M3DMatrix44f mCamera;
	modelViewMatrix.GetMatrix(mCamera);
	M3DVector4f vLightTransformed;
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

	// song and dance
	modelViewMatrix.Translate(0.0f, 0.2f, -2.5f);
	modelViewMatrix.push();	// save the translated origin
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
	modelViewMatrix.pop();	// erased the rotate

	modelViewMatrix.Rotate(yRot * -2.0f, 0.0f, 1.0f, 0.0f);
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

/**
 * Window has changed size, or has just been created. In either case, we need to use the window 
 * dimensions to set the viewport and the projection matrix.
 */
void ChangeSize(int w, int h)
{
	glViewport(0, 0, w, h);

	transformPipeline.SetMatrixStacks(modelViewMatrix, projectionMatrix);

	viewFrustum.SetPerspective(35.0f, float(w) / float(h), 1.0f, 100.0f);
	projectionMatrix.setMatrix(viewFrustum.GetProjectionMatrix());
	modelViewMatrix.identity();
}

/**
 * This function does any needed initialization on the rendering context.
 * This is the first opportunity to do any OpenGL related tasks.
 */
void SetupRC()
{
	// make sure opengl entry points are set
	glewInit();

	// Blue background
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f );

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);

	shaderManager.init();

	gltMakeTorus(torusBatch, 0.4f, 0.15f, 40, 20);
	gltMakeSphere(sphereBatch, 0.1f, 26, 13);

	// make the solid ground
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

	// draw the opengl logo
	int x = 500;
	int y = 155;
	int width = 300;
	int height = 155;
	logoBatch.Begin(GL_TRIANGLE_FAN, 4, 1);

	// Upper left hand corner
	logoBatch.MultiTexCoord2f(0, 0.0f, height);
	logoBatch.Vertex3f(x, y, 0.0);

	// Lower left hand corner
	logoBatch.MultiTexCoord2f(0, 0.0f, 0.0f);
	logoBatch.Vertex3f(x, y - height, 0.0f);

	// Lower right hand corner
	logoBatch.MultiTexCoord2f(0, width, 0.0f);
	logoBatch.Vertex3f(x + width, y - height, 0.0f);

	// Upper righ hand corner
	logoBatch.MultiTexCoord2f(0, width, height);
	logoBatch.Vertex3f(x + width, y, 0.0f);

	logoBatch.End();

	// Make 4 texture objects
	glGenTextures(4, uiTextures);

	// Load the Marble
	glBindTexture(GL_TEXTURE_2D, uiTextures[0]);
	gltLoadTextureTGA("marble.tga", GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR, GL_REPEAT);

	// Load Mars
	glBindTexture(GL_TEXTURE_2D, uiTextures[1]);
	gltLoadTextureTGA("marslike.tga", GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR, GL_CLAMP_TO_EDGE);

	// Load Moon
	glBindTexture(GL_TEXTURE_2D, uiTextures[2]);
	gltLoadTextureTGA("moonlike.tga", GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR, GL_CLAMP_TO_EDGE);

	// Load the Logo
	glBindTexture(GL_TEXTURE_RECTANGLE, uiTextures[3]);
	gltLoadTextureTGARect("OpenGL-Logo.tga", GL_NEAREST, GL_NEAREST, GL_CLAMP_TO_EDGE);

	rectReplaceShader = gltLoadShaderWithFileEx("RectReplace.vert", "RectReplace.frag", 2, 
		GLT_ATTRIBUTE_VERTEX, "vVertex", 
		GLT_ATTRIBUTE_TEXTURE0, "vTexCoord");

	locRectMVP = glGetUniformLocation(rectReplaceShader, "mvpMatrix");
	locRectTexture = glGetUniformLocation(rectReplaceShader, "rectangleImage");

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

	if (key == GLUT_KEY_UP)
		cameraFrame.MoveForward(linear);

	if (key == GLUT_KEY_DOWN)
		cameraFrame.MoveForward(-linear);

	if (key == GLUT_KEY_LEFT)
		cameraFrame.RotateWorld(angular, 0.0f, 1.0f, 0.0f);

	if (key == GLUT_KEY_RIGHT)
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
	static GLfloat vFloorColor[] = { 1.0f, 1.0f, 1.0f, 0.75f };
	static CStopWatch rotTimer;
	float yRot = rotTimer.GetElapsedSeconds() * 60.0f;

	// Clear the window with current clearing color
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	modelViewMatrix.push();

	M3DMatrix44f mCamera;
	cameraFrame.GetCameraMatrix(mCamera);
	modelViewMatrix.MultMatrix(mCamera);

	// draw the world upside down
	modelViewMatrix.push();
	modelViewMatrix.Scale(1.0f, -1.0f, 1.0f); // flips the Y axis
	modelViewMatrix.Translate(0.0f, 0.8f, 0.0f); // scootch the world down a bit...

	glFrontFace(GL_CW);
	DrawSongAndDance(yRot);
	glFrontFace(GL_CCW); // restore it

	modelViewMatrix.pop();

	// draw the solid ground
	glEnable(GL_BLEND);
	glBindTexture(GL_TEXTURE_2D, uiTextures[0]);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	shaderManager.useStockShader(GLT_SHADER_TEXTURE_MODULATE, 
		transformPipeline.GetMVPMatrix(), 
		vFloorColor, 
		0);
	floorBatch.draw();
	glDisable(GL_BLEND);

	DrawSongAndDance(yRot);

	modelViewMatrix.pop();

	// Render the overlay

	// Creating this matrix really doesn't need to be done every frame. I'll leave it here
	// so all the pertenant code is together
	M3DMatrix44f mScreenSpace;
	m3dMakeOrthographicMatrix(mScreenSpace, 0.0f, 800.0f, 0.0f, 600.0f, -1.0f, 1.0f);

	// turn blending on, and dephth testing off
	glEnable(GL_BLEND);
	glDisable(GL_DEPTH_TEST);

	glUseProgram(rectReplaceShader);
	glUniform1i(locRectTexture, 0);
	glUniformMatrix4fv(locRectMVP, 1, GL_FALSE, mScreenSpace);
	glBindTexture(GL_TEXTURE_RECTANGLE, uiTextures[3]);
	logoBatch.draw();

	// restore no blending and depth test
	glDisable(GL_BLEND);
	glEnable(GL_DEPTH_TEST);

	// Perform the buffer swap to display back buffer
	glutSwapBuffers();
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

	gltShowVersionInfo();

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

	OnExit();

	return 0;
}
