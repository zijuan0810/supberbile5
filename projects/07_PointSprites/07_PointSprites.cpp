#include "GLTool-ext.h"

#define NUM_STARS 10000

GLFrustum	viewFrustum;
GLBatch		starsBatch;

GLuint	starShader;	// The point sprite shader
GLint	locMVP;		// The location of the ModelViewProjection matrix uniform
GLint	locTimeStamp;	// The location of the time stamp
GLint	locTexture;	// The location of the  texture uniform

GLuint	starTexture;	// The star texture texture object

// Load a TGA as a 2D Texture. Completely initialize the state
static bool LoadTGATexture(const char *szFileName, GLenum minFilter, GLenum magFilter, GLenum wrapMode)
{
	// Read the texture bits
	int nWidth, nHeight, nComponents;
	GLenum eFormat;
	GLbyte* pBits = gltReadTGABits(szFileName, &nWidth, &nHeight, &nComponents, &eFormat);
	if (pBits == NULL) {
		return false;
	}

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapMode);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapMode);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minFilter);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, magFilter);

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glTexImage2D(GL_TEXTURE_2D, 0, nComponents, nWidth, nHeight, 0, eFormat, GL_UNSIGNED_BYTE, pBits);

	free(pBits);

	if (minFilter == GL_LINEAR_MIPMAP_LINEAR || minFilter == GL_LINEAR_MIPMAP_NEAREST ||
		minFilter == GL_NEAREST_MIPMAP_LINEAR || minFilter == GL_NEAREST_MIPMAP_NEAREST) {
		glGenerateMipmap(GL_TEXTURE_2D);
	}

	return true;
}

/**
 * Window has changed size, or has just been created. In either case, we need to use the window
 * dimensions to set the viewport and the projection matrix.
 */
void ChangeSize(int w, int h)
{
	// Prevent a divide by zero
	if (h == 0)
		h = 1;

	// Set Viewport to window dimensions
	glViewport(0, 0, w, h);

	viewFrustum.SetPerspective(35.0f, float(w) / float(h), 1.0f, 1000.0f);
}

/**
 * This function does any needed initialization on the rendering context.
 * This is the first opportunity to do any OpenGL related tasks.
 */
void SetupRC()
{
	// Blue background
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f );

	glEnable(GL_POINT_SPRITE);

	GLfloat fColors[4][4] = { 
		{ 1.0f, 1.0f, 1.0f, 1.0f },	// White
		{ 1.0f, 0.0f, 0.0f, 1.0f }, // Blue Stars
		{ 0.0f, 1.0f, 0.0f, 1.0f },	// Reddish
		{ 0.0f, 0.0f, 0.1f, 1.0f },	// Orange
	}; 

	// Randomly place the stars in their initial positions, and pick a random color
	starsBatch.Begin(GL_POINTS, NUM_STARS);
	for (int i = 0; i < NUM_STARS; i++) {
		int iColor = 0;		// All stars start as white

		// One in five will be blue
		if (rand() % 5 == 1)
			iColor = 1;

		// One in 50 red
		if (rand() % 50 == 1)
			iColor = 2;

		// One in 100 is amber
		if (rand() % 100 == 1)
			iColor = 3;

		starsBatch.Color4fv(fColors[iColor]);

		M3DVector3f vPosition;
		vPosition[0] = float(3000 - (rand() % 6000)) * 0.1f;
		vPosition[1] = float(3000 - (rand() % 6000)) * 0.1f;
		vPosition[2] = -float(rand() % 1000) - 1.0f;  // -1 to -1000.0f

		starsBatch.Vertex3fv(vPosition);
	}
	starsBatch.End();


	starShader = gltLoadShaderPairWithAttributes(
		"SpaceFlight.vs.glsl", "SpaceFlight.fs.glsl", 2, 
		GLT_ATTRIBUTE_VERTEX, "vVertex",
		GLT_ATTRIBUTE_COLOR, "vColor");

	locMVP = glGetUniformLocation(starShader, "mvpMatrix");
	locTexture = glGetUniformLocation(starShader, "starImage");
	locTimeStamp = glGetUniformLocation(starShader, "timeStamp");

	glGenTextures(1, &starTexture);
	glBindTexture(GL_TEXTURE_2D, starTexture);
	LoadTGATexture("Star.tga", GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR, GL_CLAMP_TO_EDGE);

	// Place the origin of the texture coordinate system at the lower-left corner of the point.
	// The default orientation for point sprites is: GL_UPPER_LEFT
	glPointParameteri(GL_POINT_SPRITE_COORD_ORIGIN, GL_LOWER_LEFT);
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
	switch (key) {
	case KEY_a:
		glEnable(GL_POINT_SPRITE);
		break;
	case KEY_b:
		glDisable(GL_POINT_SPRITE);
		break;
	default:
		break;
	}
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
	static CStopWatch timer;

	// Clear the window with current clearing color
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	// Turn on additive blending
	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_ONE);

	// Let the vertex program detemine the point size
	glEnable(GL_PROGRAM_POINT_SIZE);

	// Bind to our shader, set uniforms
	glUseProgram(starShader);
	glUniformMatrix4fv(locMVP, 1, GL_FALSE, viewFrustum.GetProjectionMatrix());
	glUniform1i(locTexture, 0);

	// fTime goes form 0.0 to 999.0 and recycles
	float fTime = timer.GetElapsedSeconds() * 10.0f;
	fTime = fmod(fTime, 999.0f);
	glUniform1f(locTexture, fTime);

	// Draw the stars
	starsBatch.Draw();

	// Perform the buffer swap to display back buffer
	glutSwapBuffers();
	glutPostRedisplay();
}

/**
 * Cleanup... such as deleting texture objects
 */
void OnExit()
{
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
