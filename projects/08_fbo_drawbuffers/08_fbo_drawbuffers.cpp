#include "GLTool-ext.h"

static GLfloat vGreen[] = { 0.0f, 1.0f, 0.0f, 1.0f };
static GLfloat vWhite[] = { 1.0f, 1.0f, 1.0f, 1.0f };
static GLfloat vLightPos[] = { 0.0f, 3.0f, 0.0f, 1.0f };
static const GLenum windowBuff[] = { GL_BACK_LEFT };
static const GLenum fboBuffs[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };

GLsizei	 screenWidth;			// Desired window or desktop width
GLsizei  screenHeight;			// Desired window or desktop height

GLboolean bFullScreen;			// Request to run full screen
GLboolean bAnimated;			// Request for continual updates


GLShaderManager		shaderManager;			// Shader Manager
GLMatrixStack		modelViewMatrix;		// Modelview Matrix
GLMatrixStack		projectionMatrix;		// Projection Matrix
GLFrustum			viewFrustum;			// View Frustum
GLGeometryTransform	transformPipeline;		// Geometry Transform Pipeline
GLFrame				cameraFrame;			// Camera frame

GLTriangleBatch		torusBatch;
GLTriangleBatch		sphereBatch;
GLBatch				floorBatch;
GLBatch             screenQuad;

GLuint				textures[3];
GLuint				processProg;
GLuint				texBO[3];
GLuint				texBOTexture;
bool                bUseFBO;
GLuint              fboName;
GLuint              depthBufferName;
GLuint				renderBufferNames[3];

SBObject            ninja;
GLuint              ninjaTex[1];

void MoveCamera(void);
void DrawWorld(GLfloat yRot);
bool LoadBMPTexture(const char *szFileName, GLenum minFilter, GLenum magFilter, GLenum wrapMode);

static float* LoadFloatData(const char *szFile, int *count)
{
	FILE* file = fopen(szFile, "r");
	if (file == nullptr) {
		return 0;
	}

	GLint lineCount = 0;
	char szFloat[1024] = { 0 };
	while (fgets(szFloat, sizeof(char) * 1024, file)) {
		lineCount++;
	}

	// Go back to begining of file
	rewind(file);

	// Allocate space for all data
	float* data = (float*)malloc(lineCount * sizeof(float));
	if (data != nullptr) {
		int idx = 0;
		while (fgets(szFloat, 1024 * sizeof(char), file)) {
			data[idx++] = (float)atof(szFloat);
		}
		count[0] = idx;
	}
	fclose(file);

	return data;
}

/**
 * Window has changed size, or has just been created. In either case, we need to use the window 
 * dimensions to set the viewport and the projection matrix.
 */
void ChangeSize(int w, int h)
{
	glViewport(0, 0, w, h);
}

/**
 * This function does any needed initialization on the rendering context.
 * This is the first opportunity to do any OpenGL related tasks.
 */
void SetupRC()
{
	// Blue background
	glClearColor(0.0f, 0.0f, 1.0f, 1.0f );

	shaderManager.init();

	glEnable(GL_DEPTH_TEST);

	ninja.loadWithFile("ninja.sbm", GLT_ATTRIBUTE_VERTEX, GLT_ATTRIBUTE_NORMAL,
		GLT_ATTRIBUTE_TEXTURE0);

	gltCreateTorus(torusBatch, 0.4f, 0.15f, 35, 35);
	gltCreateSphere(sphereBatch, 0.1f, 26, 13);

	GLfloat alpha = 0.25f;
	floorBatch.begin(GL_TRIANGLE_FAN, 4, 1);
		floorBatch.Color4f(0.0f, 1.0f, 0.0f, alpha);
		floorBatch.MultiTexCoord2f(0, 0.0f, 0.0f);
		floorBatch.Normal3f(0.0, 1.0f, 0.0f);
		floorBatch.Vertex3f(-20.0f, -0.41f, 20.0f);

		floorBatch.Color4f(0.0f, 1.0f, 0.0f, alpha);
		floorBatch.MultiTexCoord2f(0, 10.0f, 0.0f);
		floorBatch.Normal3f(0.0, 1.0f, 0.0f);
		floorBatch.Vertex3f(20.0f, -0.41f, 20.0f);

		floorBatch.Color4f(0.0f, 1.0f, 0.0f, alpha);
		floorBatch.MultiTexCoord2f(0, 10.0f, 10.0f);
		floorBatch.Normal3f(0.0, 1.0f, 0.0f);
		floorBatch.Vertex3f(20.0f, -0.41f, -20.0f);

		floorBatch.Color4f(0.0f, 1.0f, 0.0f, alpha);
		floorBatch.MultiTexCoord2f(0, 0.0f, 10.0f);
		floorBatch.Normal3f(0.0, 1.0f, 0.0f);
		floorBatch.Vertex3f(-20.0f, -0.41f, -20.0f);
	floorBatch.end();

	glGenTextures(1, textures);
	glBindTexture(GL_TEXTURE_2D, textures[0]);
	gltLoadTextureBMP("marble.bmp", GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR, GL_REPEAT);

	glGenFramebuffers(1, &fboName);

	// Create depth renderbuffer
	glGenRenderbuffers(1, &depthBufferName);
	glBindRenderbuffer(GL_RENDERBUFFER, depthBufferName);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT32, screenWidth, screenHeight);

	// Create 3 color renderbuffers
	glGenRenderbuffers(3, renderBufferNames);
	for (int i = 0; i < 3; ++i) {
		glBindRenderbuffer(GL_RENDERBUFFER, renderBufferNames[i]);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA8, screenWidth, screenHeight);
	}

	// Attach all 4 renderbuffers to FBO
	glBindRenderbuffer(GL_DRAW_FRAMEBUFFER, fboName);
	glFramebufferRenderbuffer(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthBufferName);
	glFramebufferRenderbuffer(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, renderBufferNames[0]);
	glFramebufferRenderbuffer(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_RENDERBUFFER, renderBufferNames[1]);
	glFramebufferRenderbuffer(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_RENDERBUFFER, renderBufferNames[2]);

	// See bind frag location in Chapter 9
	processProg = gltLoadShaderWithFileEx("multibuffer.vs.glsl", "multibuffer_frag_location.fs.glsl", 3,
		GLT_ATTRIBUTE_VERTEX, "vVertex",
		GLT_ATTRIBUTE_NORMAL, "vNormal",
		GLT_ATTRIBUTE_TEXTURE0, "texCoord0");
	glBindFragDataLocation(processProg, 0, "oStraightColor");
	glBindFragDataLocation(processProg, 1, "oGreyscale");
	glBindFragDataLocation(processProg, 2, "oLumAdjColor");

	// Create 3 new buffer objects
	glGenBuffers(3, texBO);
	glGenTextures(1, &texBOTexture);

	// Load first texBO with a tangent-like curve, 1024 values
	int count = 0;
	//float* fileData = LoadFloatData("LumTan.data", &count);
	shared_ptr<float> fileData(LoadFloatData("LumTan.data", &count));
	if (count > 0) {
		glBindBuffer(GL_TEXTURE_BUFFER_ARB, texBO[0]);
		glBufferData(GL_TEXTURE_BUFFER_ARB, count * sizeof(float), (float*)fileData.get(), GL_STATIC_DRAW);
	}

	// Load second texBO with a sine-like curve, 1024 values
	fileData = make_shared<float>(LoadFloatData("LumSin.data", &count));
	if (count > 0) {
		glBindBuffer(GL_TEXTURE_BUFFER_ARB, texBO[1]);
		glBufferData(GL_TEXTURE_BUFFER_ARB, count * sizeof(float), (float*)fileData.get(), GL_STATIC_DRAW);
	}

	// Load third texBO with a linear curve, 1024 values
	fileData = make_shared<float>(LoadFloatData("LumSin.data", &count));
	if (count > 0) {
		glBindBuffer(GL_TEXTURE_BUFFER_ARB, texBO[2]);
		glBufferData(GL_TEXTURE_BUFFER_ARB, count * sizeof(float), (float)fileData.get(), GL_STATIC_DRAW);
	}

	// Load the Ta ramp first
	glBindBuffer(GL_TEXTURE_BUFFER, 0);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_BUFFER_ARB, texBOTexture);
	glTexBufferARB(GL_TEXTURE_BUFFER_ARB, GL_R32F, texBO[0]);
	glActiveTexture(GL_TEXTURE0);

	// Reset framebuffer binding
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

	// Make sure all went well
	gltCheckErrors();
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

	GLfloat vRed[] = { 1.0f, 0.0f, 0.0f, 1.0f };
	shaderManager.useStockShader(GLT_SHADER_IDENTITY, vRed);

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
