#include "GLTool-ext.h"

using namespace std;

GLShaderManager	shaderManager;
GLFrustum viewFrustum;
GLBatch smallStarBatch;
GLBatch mediumStarBatch;
GLBatch largeStarBatch;
GLBatch mountainRangeBatch;
GLBatch moonBatch;

GLuint  starTexture;
GLuint	starFieldShader;	// The point sprite shader
GLint	locMVP;				// The location of the ModelViewProjection matrix uniform
GLint	locStarTexture;		// The location of the  texture uniform


GLuint  moonTexture;
GLuint  moonShader;
GLint   locMoonMVP;
GLint   locMoonTexture;
GLint	locMoonTime;

GLint   locTimeStamp;       // The location of the time stamp


// Array of small stars
#define SMALL_STARS     100
#define MEDIUM_STARS     40
#define LARGE_STARS      15

#define SCREEN_X        800
#define SCREEN_Y        600

// Load a TGA as a 2D Texture. Completely initialize the state
bool LoadTGATexture(const char *szFileName, GLenum minFilter, GLenum magFilter, GLenum wrapMode)
{
	GLbyte *pBits;
	int nWidth, nHeight, nComponents;
	GLenum eFormat;

	// Read the texture bits
	pBits = gltReadTGABits(szFileName, &nWidth, &nHeight, &nComponents, &eFormat);
	if (pBits == NULL)
		return false;

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapMode);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapMode);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minFilter);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, magFilter);

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glTexImage2D(GL_TEXTURE_2D, 0, nComponents, nWidth, nHeight, 0,
		eFormat, GL_UNSIGNED_BYTE, pBits);

	free(pBits);

	if (minFilter == GL_LINEAR_MIPMAP_LINEAR ||
		minFilter == GL_LINEAR_MIPMAP_NEAREST ||
		minFilter == GL_NEAREST_MIPMAP_LINEAR ||
		minFilter == GL_NEAREST_MIPMAP_NEAREST)
		glGenerateMipmap(GL_TEXTURE_2D);

	return true;
}

/**
* Called to draw scene
*/
void RenderScene(void)
{
	static CStopWatch timer;

	// Clear the window with current clearing color
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	// Everything is white
	GLfloat vWhite[] = { 1.0f, 1.0f, 1.0f, 1.0f };

	glBindTexture(GL_TEXTURE_2D, starTexture);
	glUseProgram(starFieldShader);
	glUniformMatrix4fv(locMVP, 1, GL_FALSE, viewFrustum.GetProjectionMatrix());
	glUniform1i(locStarTexture, 0);

	// draw small stars
	glPointSize(4.0f);
	smallStarBatch.draw();

	// draw meduim sized stars
	glPointSize(8.0f);
	mediumStarBatch.draw();

	// draw larget sized stars
	glPointSize(12.0f);
	largeStarBatch.draw();

	// draw distant horizon
	shaderManager.useStockShader(GLT_SHADER_FLAT, viewFrustum.GetProjectionMatrix(), vWhite);
	glLineWidth(3.5f);
	mountainRangeBatch.draw();

	// draw the "moon"
	glBindTexture(GL_TEXTURE_2D_ARRAY, moonTexture);
	glUseProgram(moonShader);
	glUniformMatrix4fv(locMoonMVP, 1, GL_FALSE, viewFrustum.GetProjectionMatrix());
	glUniform1i(locMoonTexture, 0);

	// fTime goes from 0.0 to 28.0 and recycles
	float fTime = timer.delta();
	fTime = fmod(fTime, 28.0f);
	glUniform1f(locTimeStamp, fTime);

	moonBatch.draw();

	// Perform the buffer swap to display back buffer
	glutSwapBuffers();
	glutPostRedisplay();
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

	// Establish clipping volume (left, right, bottom, top, near, far)
	viewFrustum.SetOrthographic(0.0f, SCREEN_X, 0.0f, SCREEN_Y, -1.0f, 1.0f);
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

	// SMALL_STARS is the largest batch we are going to need
	M3DVector3f vVerts[SMALL_STARS] = { 0.0f };
	for (int i = 0; i < SMALL_STARS; ++i) {
		//vVerts[i][0] = (GLfloat)(rand() % SCREEN_X);
		//vVerts[i][1] = (GLfloat)(rand() % (SCREEN_Y - 100)) + 100.0f;
		//vVerts[i][2] = 0.0f;
	}

	// Populate small star list
	smallStarBatch.begin(GL_POINTS, SMALL_STARS);
	for (int i = 0; i < SMALL_STARS; ++i) {
		vVerts[i][0] = (GLfloat)(rand() % SCREEN_X);
		vVerts[i][1] = (GLfloat)(rand() % (SCREEN_Y - 100)) + 100.0f;
		vVerts[i][2] = 0.0f;
	}
	smallStarBatch.CopyVertexData3f(vVerts);
	smallStarBatch.end();

	// Populate medium star list
	mediumStarBatch.begin(GL_POINTS, MEDIUM_STARS);
	for (int i = 0; i < MEDIUM_STARS; ++i) {
		vVerts[i][0] = (GLfloat)(rand() % SCREEN_X);
		vVerts[i][1] = (GLfloat)(rand() % (SCREEN_Y - 100)) + 100.0f;
		vVerts[i][2] = 0.0f;
	}
	mediumStarBatch.CopyVertexData3f(vVerts);
	mediumStarBatch.end();

	// Populate large star list
	largeStarBatch.begin(GL_POINTS, LARGE_STARS);
	for (int i = 0; i < LARGE_STARS; ++i) {
		vVerts[i][0] = (GLfloat)(rand() % SCREEN_X);
		vVerts[i][1] = (GLfloat)(rand() % (SCREEN_Y - 100)) + 100.0f;
		vVerts[i][2] = 0.0f;
	}
	largeStarBatch.CopyVertexData3f(vVerts);
	largeStarBatch.end();

	// Populate mountains
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

	// Populate the moon
	GLfloat x = 700.0f;     // Location and radius of moon
	GLfloat y = 500.0f;
	GLfloat r = 50.0f;
	GLfloat angle = 0.0f;   // Another looping variable
	moonBatch.begin(GL_TRIANGLE_FAN, 4, 1);
	moonBatch.MultiTexCoord2f(0, 0.0f, 0.0f);
	moonBatch.Vertex3f(x - r, y - r, 0.0f);

	moonBatch.MultiTexCoord2f(0, 1.0f, 0.0f);
	moonBatch.Vertex3f(x + r, y - r, 0.0f);

	moonBatch.MultiTexCoord2f(0, 1.0f, 1.0f);
	moonBatch.Vertex3f(x + r, y + r, 0.0f);

	moonBatch.MultiTexCoord2f(0, 0.0f, 1.0f);
	moonBatch.Vertex3f(x - r, y + r, 0.0f);
	moonBatch.end();


	// Turn on line antialiasing, and give hint to to the best job possible
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);
	glEnable(GL_LINE_SMOOTH);
	glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);

	// Shader operation
	starFieldShader = gltLoadShaderWithFileEx("StarField.vs.glsl", "StarField.fs.glsl", 1, 
		GLT_ATTRIBUTE_VERTEX, "vVertex");
	locMVP = glGetUniformLocation(starFieldShader, "mvpMatrix");
	locStarTexture = glGetUniformLocation(starFieldShader, "starImage");

	moonShader = gltLoadShaderWithFileEx("MoonShader.vs.glsl", "MoonShader.fs.glsl", 2, 
		GLT_ATTRIBUTE_VERTEX, "vVertex", 
		GLT_ATTRIBUTE_TEXTURE0, "vTexCoords");
	locMoonMVP = glGetUniformLocation(moonShader, "mvpMatrix");
	locMoonTexture = glGetUniformLocation(moonShader, "moonImage");
	locMoonTime = glGetUniformLocation(moonShader, "fTime");

	// Texture operation
	glGenTextures(1, &starTexture);
	glBindTexture(GL_TEXTURE_2D, starTexture);
	LoadTGATexture("Star.tga", GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR, GL_CLAMP_TO_EDGE);

	// Texture Array
	glGenTextures(1, &moonTexture);
	glBindTexture(GL_TEXTURE_2D_ARRAY, moonTexture);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_RGBA, 64, 64, 30, 0, GL_BGRA,
		GL_UNSIGNED_BYTE, nullptr);
	for (int i = 0; i < 29; ++i) {
		char file_name[32] = { 0 };
		sprintf(file_name, "moon%02d.tga", i);

		int nWidth, nHeight, nComponents;
		GLenum eFormat;

		// Read the texture bits
		//shared_ptr<GLbyte> bits(gltReadTGABits(file_name, &nWidth, &nHeight, &nComponents, &eFormat));
		GLbyte* pBits = gltReadTGABits(file_name, &nWidth, &nHeight, &nComponents, &eFormat);
		glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, i, nWidth, nHeight, 1, GL_BGRA, 
			GL_UNSIGNED_BYTE, pBits);
		free(pBits);
	}
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
