#include "GLTool-ext.h"

static GLfloat vGreen[] = { 0.0f, 1.0f, 0.0f, 1.0f };
static GLfloat vWhite[] = { 1.0f, 1.0f, 1.0f, 1.0f };
static GLfloat vLightPos[] = { 0.0f, 3.0f, 0.0f, 1.0f };

GLsizei	 screenWidth;			// Desired window or desktop width
GLsizei  screenHeight;			// Desired window or desktop height

GLboolean bFullScreen;			// Request to run full screen
GLboolean bAnimated;			// Request for continual updates


GLShaderManager		shaderManager;			// Shader Manager
GLMatrixStack		modelViewMatrix;		// Modelview Matrix
GLMatrixStack		projectionMatrix;		// Projection Matrix
M3DMatrix44f        orthoMatrix;
GLFrustum			viewFrustum;			// View Frustum
GLGeometryTransform	transformPipeline;		// Geometry Transform Pipeline
GLFrame				cameraFrame;			// Camera frame

GLTriangleBatch		torusBatch;
GLBatch				floorBatch;
GLBatch             screenQuad;

GLuint	textures[1];
GLuint	blurTextures[6];
GLuint	pixBuffObjs[1];
GLuint	curBlurTarget;
bool		bUsePBOPath;
GLfloat	speedFactor;
GLuint	blurProg;
void		*pixelData;

void MoveCamera(void);
void DrawWorld(GLfloat yRot, GLfloat xPos);
//bool LoadBMPTexture(const char *szFileName, GLenum minFilter, GLenum magFilter, GLenum wrapMode);

void SetupBlurProg(void);

// return 1-6 for blur texture units
// curPixBuf is always between 0 and 5
void AdvanceBlurTaget(){ curBlurTarget = ((curBlurTarget + 1) % 6); }
GLuint GetBlurTarget0(){ return (1 + ((curBlurTarget + 5) % 6)); }
GLuint GetBlurTarget1(){ return (1 + ((curBlurTarget + 4) % 6)); }
GLuint GetBlurTarget2(){ return (1 + ((curBlurTarget + 3) % 6)); }
GLuint GetBlurTarget3(){ return (1 + ((curBlurTarget + 2) % 6)); }
GLuint GetBlurTarget4(){ return (1 + ((curBlurTarget + 1) % 6)); }
GLuint GetBlurTarget5(){ return (1 + ((curBlurTarget) % 6)); }

void updateFrameCount()
{
	static int iFrames = 0; // Frame count
	static CStopWatch frameTimer; // Render time

	// Reset the stopwatch on first time
	if (iFrames == 0) {
		frameTimer.Reset();
		iFrames++;
	}

	iFrames++; // Increment the frame count

	// Do periodic frame rate calulation
	if (iFrames == 101) {
		float fps = 100.0f / frameTimer.GetElapsedSeconds();
		if (bUsePBOPath) {
			log("Pix_buffs - Using PBOs  %.1f fps", fps);
		}
		else {
			log("Pix_buffs - Using Client mem copies  %.1f fps", fps);
		}

		frameTimer.Reset();
		iFrames = 1;
	}
}

/**
 * Window has changed size, or has just been created. In either case, we need to use the window 
 * dimensions to set the viewport and the projection matrix.
 */
void ChangeSize(int w, int h)
{
	glViewport(0, 0, w, h);

	transformPipeline.SetMatrixStacks(modelViewMatrix, projectionMatrix);
	viewFrustum.SetPerspective(35.0f, float(w) / float(h), 1.0f, 1000.0f);
	projectionMatrix.LoadMatrix(viewFrustum.GetProjectionMatrix());
	modelViewMatrix.LoadIdentity();

	// Update screen sizes
	screenWidth = w;
	screenHeight = h;

	// Reset screen aligned quad
	gltGenerateOrtho2DMat(screenWidth, screenHeight, orthoMatrix, screenQuad);

	free(pixelData);
	GLuint pixelDataSize = screenWidth * screenHeight * 3 * sizeof(unsigned int);
	pixelData = (void*)malloc(pixelDataSize);
	memset(pixelData, 0, pixelDataSize);

	// Resize PBOs
	glBindBuffer(GL_PIXEL_PACK_BUFFER, pixBuffObjs[0]);
	glBufferData(GL_PIXEL_PACK_BUFFER, pixelDataSize, pixelData, GL_DYNAMIC_COPY);

	gltCheckErrors();
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

	gltMakeTorus(torusBatch, 0.4f, 0.15f, 35, 35);

	GLfloat alpha = 0.25f;
	floorBatch.Begin(GL_TRIANGLE_FAN, 4, 1);
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
	floorBatch.End();

	glGenTextures(1, textures);
	glBindTexture(GL_TEXTURE_2D, textures[0]);
	gltLoadTextureTGA("marble.bmp", GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR, GL_REPEAT);

	// Create blur program
	blurProg = gltLoadShaderPairWithAttributes("blur.vs.glsl", "blur.fs.glsl", 2,
		GLT_ATTRIBUTE_VERTEX, "vVertex",
		GLT_ATTRIBUTE_TEXTURE0, "texCoord0");

	// Create blur textures
	glGenTextures(6, blurTextures);

	// XXX I don't think this is necessary. Should set texture data to NULL
	// Allocate a pixel buffer to initialize textures and PBOs
	GLuint pixelDataSize = screenWidth * screenHeight * 3 * sizeof(unsigned int); // XXX This should be unsigned byte
	void* data = (void*)malloc(pixelDataSize);
	memset(data, 0x00, pixelDataSize);

	// Setup 6 texture units for blur effect
	// Initialize texture data
	for (int i = 0; i < 6; ++i) {
		glActiveTexture(GL_TEXTURE1 + i);
		glBindTexture(GL_TEXTURE_2D, blurTextures[i]);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, screenWidth, screenHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
	}

	// Alloc space for copying pixels so we don't call malloc on every frame
	glGenBuffers(1, pixBuffObjs);
	glBindBuffer(GL_PIXEL_PACK_BUFFER, pixBuffObjs[0]);
	glBufferData(GL_PIXEL_PACK_BUFFER, pixelDataSize, pixelData, GL_DYNAMIC_COPY);
	glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);

	// Create geometry and a matrix for screen aligned drawing
	gltGenerateOrtho2DMat(screenWidth, screenHeight, orthoMatrix, screenQuad);

	// Make sure all went well
	gltCheckErrors();
}

/**
 * Respond to arrow keys by moving the camera frame of reference
 */
void SpecialKeys(int key, int x, int y)
{
	static CStopWatch cameraTimer;
	float fTime = cameraTimer.GetElapsedSeconds();
	float linear = fTime * 12.0f;
	cameraTimer.Reset();

	// Alternate between PBOs and local memory when 'P' is pressed
	if (key == KEY_P || key == 'p') {
		bUsePBOPath = (bUsePBOPath) ? GL_FALSE : GL_TRUE;
	}

	// Speed up movement
	if (key == '+') {
		speedFactor += linear / 2.0f;
		if (speedFactor > 6.0f) {
			speedFactor = 6.0f;
		}
	}

	// Slow down movement
	if (key == '-') {
		speedFactor -= linear / 2.0f;
		if (speedFactor < 0.5f) {
			speedFactor = 0.5f;
		}
	}
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
	static CStopWatch animationTimer;
	static float totalTime = 6; // To go back and forth
	static float halfTotalTime = totalTime / 2;
	float seconds = animationTimer.GetElapsedSeconds() * speedFactor;
	float xPos = 0;

	// Calculate the next postion of the moving object
	// First perform a mod-like operation on the time as a float
	while (seconds > totalTime)
		seconds -= totalTime;

	// Move object position, if it's gone half way across
	// start bringing it back
	if (seconds < halfTotalTime) {
		xPos = seconds - halfTotalTime*0.5f;
	}
	else {
		xPos = totalTime - seconds - halfTotalTime*0.5f;
	}

	// First draw world to screen
	modelViewMatrix.PushMatrix();
	M3DMatrix44f mCamera;
	cameraFrame.GetCameraMatrix(mCamera);
	modelViewMatrix.MultMatrix(mCamera);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, textures[0]); // Marble
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	shaderManager.UseStockShader(GLT_SHADER_TEXTURE_MODULATE, transformPipeline.GetMVPMatrix(),
		vWhite, 0);

	floorBatch.Draw();
	DrawWorld(0.0f, xPos);
	modelViewMatrix.PopMatrix();

	if (bUsePBOPath) {
		// First bind the PBO as the pack buffer, then read the pixels directly to the PBO
		glBindBuffer(GL_PIXEL_PACK_BUFFER, pixBuffObjs[0]);
		glReadPixels(0, 0, screenWidth, screenHeight, GL_RGB, GL_UNSIGNED_BYTE, NULL);
		glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);

		// Next bind the PBO as the unpack buffer, 
		// then pus the pixels strainght into the texture
		glBindBuffer(GL_PIXEL_UNPACK_BUFFER, pixBuffObjs[0]);

		// Setup texture unit for new blur, this gets incremented every frame
		glActiveTexture(GL_TEXTURE0 + GetBlurTarget0());
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, screenWidth, screenHeight, 0, GL_RGB,
			GL_UNSIGNED_BYTE, NULL);
		glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
	}
	else {
		// Grab the screen pixels and copy into local memory
		glReadPixels(0, 0, screenWidth, screenHeight, GL_RGB, GL_UNSIGNED_BYTE, pixelData);

		// Push pixels from client memory into texture
		// Setup texture unit for new blur, this gets imcremented every frame
		glActiveTexture(GL_TEXTURE0 + GetBlurTarget0());
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, screenWidth, screenHeight, 0, GL_RGB, 
			GL_UNSIGNED_BYTE, pixelData);
	}

	// Draw full screen quad with blur shader and all blur textures
	projectionMatrix.PushMatrix();
		projectionMatrix.LoadIdentity();
		projectionMatrix.LoadMatrix(orthoMatrix);
		modelViewMatrix.PushMatrix();
		modelViewMatrix.LoadIdentity();
			glDisable(GL_DEPTH_TEST);
			SetupBlurProg();
			screenQuad.Draw();
			glEnable(GL_DEPTH_TEST);
		modelViewMatrix.PopMatrix();
	projectionMatrix.PopMatrix();

	// Move to the next blur texture for the next frame
	AdvanceBlurTaget();

	// Perform the buffer swap to display back buffer
	glutSwapBuffers();
	glutPostRedisplay();

	updateFrameCount();
}

/**
 * Cleanup... such as deleting texture objects
 */
void OnExit()
{
	// Make sure default FBO is bound
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
	glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);

	// Cleanup textures
	for (int i = 0; i < 7; ++i) {
		glActiveTexture(GL_TEXTURE0 + i);
		glBindTexture(GL_TEXTURE_2D, 0);
	}

	// Now delete detached textures
	glDeleteTextures(1, textures);
	glDeleteTextures(6, blurTextures);

	// Delete FBO
	glDeleteBuffers(1, pixBuffObjs);
}

/*
 * Load and setup program for blur effect
 */
void SetupBlurProg()
{
	// Set the blur programe as the current one
	glUseProgram(blurProg);

	// Set MVP matrix
	glUniformMatrix4fv(glGetUniformLocation(blurProg, "mvpMatrix"), 1, GL_FALSE, transformPipeline.GetMVPMatrix());

	// Setup the texture units for the blur targets
	glUniform1i(glGetUniformLocation(blurProg, "textureUnit0"), GetBlurTarget0());
	glUniform1i(glGetUniformLocation(blurProg, "textureUnit1"), GetBlurTarget1());
	glUniform1i(glGetUniformLocation(blurProg, "textureUnit2"), GetBlurTarget2());
	glUniform1i(glGetUniformLocation(blurProg, "textureUnit3"), GetBlurTarget3());
	glUniform1i(glGetUniformLocation(blurProg, "textureUnit4"), GetBlurTarget4());
	glUniform1i(glGetUniformLocation(blurProg, "textureUnit5"), GetBlurTarget5());
}

/*
 * Draw the scene
 */
void DrawWorld(GLfloat yRot, GLfloat xPos)
{
	M3DMatrix44f mCamera;
	modelViewMatrix.GetMatrix(mCamera);

	// Need light position relative to the camera
	M3DVector4f vLightTransformed;
	m3dTransformVector4(vLightTransformed, vLightPos, mCamera);

	// Draw stuff relative to the camera
	modelViewMatrix.PushMatrix();
	modelViewMatrix.Translate(0.0f, 0.2f, -2.5f);
	modelViewMatrix.Translate(xPos, 0.0f, 0.0f);
	modelViewMatrix.Rotate(yRot, 0.0f, 1.0f, 0.0f);

	shaderManager.UseStockShader(GLT_SHADER_POINT_LIGHT_DIFF, modelViewMatrix.GetMatrix(),
		transformPipeline.GetMVPMatrix(), vLightTransformed, vGreen, 0);
	torusBatch.Draw();
	modelViewMatrix.PopMatrix();
}

/**
 * Main entry point for GLUT based programs
 */
int main(int argc, char* argv[])
{
    screenWidth  = 800;
    screenHeight = 600;
    bFullScreen = false; 
    bAnimated   = true;
    bUsePBOPath = false;
    blurProg    = 0;
    speedFactor = 1.0f;

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
