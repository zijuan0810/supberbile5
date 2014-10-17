#include "GLTool-ext.h"

#pragma warning( disable : 4305 )

static GLfloat vLtBlue[] = { 0.00f, 0.00f, 1.00f, 0.90f };
static GLfloat vLtPink[] = { 0.40f, 0.00f, 0.20f, 0.50f };
static GLfloat vLtYellow[] = { 0.98f, 0.96f, 0.14f, 0.30f };
static GLfloat vLtMagenta[] = { 0.83f, 0.04f, 0.83f, 0.70f };
static GLfloat vLtGreen[] = { 0.05f, 0.98f, 0.14f, 0.30f };

static GLfloat vGrey[] = { 0.5f, 0.5f, 0.5f, 1.0f };

#define USER_OIT   1 
#define USER_BLEND 2

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

GLTriangleBatch		bckgrndCylBatch;
GLTriangleBatch		diskBatch;
GLBatch				glass1Batch;
GLBatch				glass2Batch;
GLBatch				glass3Batch;
GLBatch				glass4Batch;
GLBatch             screenQuad;
M3DMatrix44f        orthoMatrix;
GLfloat             worldAngle;

GLint               blendMode;
GLint               mode;

GLuint              msFBO;
GLuint              textures[2];
GLuint		    msTexture[1];
GLuint              depthTextureName;
GLuint              msResolve;
GLuint              oitResolve;
GLuint              flatBlendProg;

void DrawWorld();
bool LoadBMPTexture(const char *szFileName, GLenum minFilter, GLenum magFilter, GLenum wrapMode);
void GenerateOrtho2DMat(GLuint imageWidth, GLuint imageHeight);
void SetupResolveProg();
void SetupOITResolveProg();

//////////////////////////////////////////////////////////////////////////////////////////////////////
// Create a matrix that maps geometry to the screen. 1 unit in the x directionequals one pixel 
// of width, same with the y direction.
//
void GenerateOrtho2DMat(GLuint imageWidth, GLuint imageHeight)
{
	float right = (float)imageWidth;
	float quadWidth = right;
	float left = 0.0f;
	float top = (float)imageHeight;
	float quadHeight = top;
	float bottom = 0.0f;

	// set ortho matrix
	orthoMatrix[0] = (float)(2 / (right));
	orthoMatrix[1] = 0.0;
	orthoMatrix[2] = 0.0;
	orthoMatrix[3] = 0.0;

	orthoMatrix[4] = 0.0;
	orthoMatrix[5] = (float)(2 / (top));
	orthoMatrix[6] = 0.0;
	orthoMatrix[7] = 0.0;

	orthoMatrix[8] = 0.0;
	orthoMatrix[9] = 0.0;
	orthoMatrix[10] = (float)(-2 / (1.0 - 0.0));
	orthoMatrix[11] = 0.0;

	orthoMatrix[12] = -1.0f;
	orthoMatrix[13] = -1.0f;
	orthoMatrix[14] = -1.0f;
	orthoMatrix[15] = 1.0;

	// set screen quad vertex array
	screenQuad.Reset();
	screenQuad.begin(GL_TRIANGLE_STRIP, 4, 1);
	screenQuad.Color4f(0.0f, 1.0f, 0.0f, 1.0f);
	screenQuad.MultiTexCoord2f(0, 0.0f, 0.0f);
	screenQuad.Vertex3f(0.0f, 0.0f, 0.0f);

	screenQuad.Color4f(0.0f, 1.0f, 0.0f, 1.0f);
	screenQuad.MultiTexCoord2f(0, 1.0f, 0.0f);
	screenQuad.Vertex3f(right, 0.0f, 0.0f);

	screenQuad.Color4f(0.0f, 1.0f, 0.0f, 1.0f);
	screenQuad.MultiTexCoord2f(0, 0.0f, 1.0f);
	screenQuad.Vertex3f(0.0f, top, 0.0f);

	screenQuad.Color4f(0.0f, 1.0f, 0.0f, 1.0f);
	screenQuad.MultiTexCoord2f(0, 1.0f, 1.0f);
	screenQuad.Vertex3f(right, top, 0.0f);
	screenQuad.end();
}

void SetupResolveProg()
{
	glUseProgram(msResolve);

	// Set projection matrix
	glUniformMatrix4fv(glGetUniformLocation(msResolve, "pMatrix"),
		1, GL_FALSE, transformPipeline.GetProjectionMatrix());

	// Set MVP matrix
	glUniformMatrix4fv(glGetUniformLocation(msResolve, "mvMatrix"),
		1, GL_FALSE, transformPipeline.GetModelViewMatrix());

	// Now setup the right textures
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, msTexture[0]);
	glUniform1i(glGetUniformLocation(msResolve, "origImage"), 0);

	glUniform1i(glGetUniformLocation(msResolve, "sampleCount"), 8);

	glActiveTexture(GL_TEXTURE0);

	gltCheckErrors(msResolve);
}

void SetupOITResolveProg()
{
	glUseProgram(oitResolve);

	// Set projection matrix
	glUniformMatrix4fv(glGetUniformLocation(oitResolve, "pMatrix"),
		1, GL_FALSE, transformPipeline.GetProjectionMatrix());

	// Set MVP matrix
	glUniformMatrix4fv(glGetUniformLocation(oitResolve, "mvMatrix"),
		1, GL_FALSE, transformPipeline.GetModelViewMatrix());

	// Now setup the right textures
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, msTexture[0]);
	glUniform1i(glGetUniformLocation(oitResolve, "origImage"), 0);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, depthTextureName);
	glUniform1i(glGetUniformLocation(oitResolve, "origDepth"), 1);

	glUniform1f(glGetUniformLocation(oitResolve, "sampleCount"), 8);

	glActiveTexture(GL_TEXTURE0);
	gltCheckErrors(oitResolve);
}

///////////////////////////////////////////////////////////////////////////////
// Draw the scene 
// 
void DrawWorld()
{
	modelViewMatrix.moveTo(0.0f, 0.8f, 0.0f);
	modelViewMatrix.push();
	modelViewMatrix.moveTo(-0.3f, 0.f, 0.0f);
	modelViewMatrix.scaleTo(0.40, 0.8, 0.40);
	modelViewMatrix.rotateTo(50.0, 0.0, 10.0, 0.0);
	glSampleMaski(0, 0x02);
	shaderManager.useStockShader(GLT_SHADER_FLAT, transformPipeline.GetMVPMatrix(), vLtYellow);
	glass1Batch.draw();
	modelViewMatrix.pop();

	modelViewMatrix.push();
	modelViewMatrix.moveTo(0.4f, 0.0f, 0.0f);
	modelViewMatrix.scaleTo(0.5, 0.8, 1.0);
	modelViewMatrix.rotateTo(-20.0, 0.0, 1.0, 0.0);
	glSampleMaski(0, 0x04);
	shaderManager.useStockShader(GLT_SHADER_FLAT, transformPipeline.GetMVPMatrix(), vLtGreen);
	glass2Batch.draw();
	modelViewMatrix.pop();

	modelViewMatrix.push();
	modelViewMatrix.moveTo(1.0f, 0.0f, -0.6f);
	modelViewMatrix.scaleTo(0.3, 0.9, 1.0);
	modelViewMatrix.rotateTo(-40.0, 0.0, 1.0, 0.0);
	glSampleMaski(0, 0x08);
	shaderManager.useStockShader(GLT_SHADER_FLAT, transformPipeline.GetMVPMatrix(), vLtMagenta);
	glass3Batch.draw();
	modelViewMatrix.pop();

	modelViewMatrix.push();
	modelViewMatrix.moveTo(-0.8f, 0.0f, -0.60f);
	modelViewMatrix.scaleTo(0.6, 0.9, 0.40);
	modelViewMatrix.rotateTo(60.0, 0.0, 1.0, 0.0);
	glSampleMaski(0, 0x10);
	shaderManager.useStockShader(GLT_SHADER_FLAT, transformPipeline.GetMVPMatrix(), vLtBlue);
	glass4Batch.draw();
	modelViewMatrix.pop();

	modelViewMatrix.push();
	modelViewMatrix.moveTo(0.1f, 0.0f, 0.50f);
	modelViewMatrix.scaleTo(0.4, 0.9, 0.4);
	modelViewMatrix.rotateTo(205.0, 0.0, 1.0, 0.0);
	glSampleMaski(0, 0x20);
	shaderManager.useStockShader(GLT_SHADER_FLAT, transformPipeline.GetMVPMatrix(), vLtPink);
	glass4Batch.draw();
	modelViewMatrix.pop();
}



/**
 * Window has changed size, or has just been created. In either case, we need to use the window 
 * dimensions to set the viewport and the projection matrix.
 */
void ChangeSize(int w, int h)
{
	glViewport(0, 0, w, h);
	transformPipeline.setMatrixStacks(modelViewMatrix, projectionMatrix);

	viewFrustum.setPerspective(35.0f, float(w) / float(h), 1.0f, 100.0f);
	projectionMatrix.setMatrix(viewFrustum.GetProjectionMatrix());
	modelViewMatrix.identity();

	GenerateOrtho2DMat(w, h);

	screenWidth = w;
	screenHeight = h;

	// resize texture
	glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, depthTextureName);
	glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, 8, GL_DEPTH_COMPONENT24, w, h, GL_FALSE);

	glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, msTexture[0]);
	glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, 8, GL_RGBA8, w, h, GL_FALSE);
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

	gltCreateCylinder(bckgrndCylBatch, 4.0f, 4.0f, 5.2f, 1024, 1);
	gltCreateDisk(diskBatch, 0.0f, 1.5f, 40, 10);

	glass1Batch.begin(GL_TRIANGLE_FAN, 4, 1);
	glass1Batch.Vertex3f(-1.0f, -1.0f, 0.0f);
	glass1Batch.Vertex3f(1.0f, -1.0f, 0.0f);
	glass1Batch.Vertex3f(1.0f, 1.0f, 0.0f);
	glass1Batch.Vertex3f(-1.0f, 1.0f, 0.0f);
	glass1Batch.end();

	glass2Batch.begin(GL_TRIANGLE_FAN, 4, 1);
	glass2Batch.Vertex3f(0.0f, 1.0f, 0.0f);
	glass2Batch.Vertex3f(1.0f, 0.0f, 0.0f);
	glass2Batch.Vertex3f(0.0f, -1.0f, 0.0f);
	glass2Batch.Vertex3f(-1.0f, 0.0f, 0.0f);
	glass2Batch.end();

	glass3Batch.begin(GL_TRIANGLE_FAN, 3, 1);
	glass3Batch.Vertex3f(0.0f, 1.0f, 0.0f);
	glass3Batch.Vertex3f(1.0f, -1.0f, 0.0f);
	glass3Batch.Vertex3f(-1.0f, -1.0f, 0.0f);
	glass3Batch.end();

	glass4Batch.begin(GL_TRIANGLE_FAN, 4, 1);
	glass4Batch.Vertex3f(-1.0f, 1.0f, 0.0f);
	glass4Batch.Vertex3f(1.0f, 0.5f, 0.0f);
	glass4Batch.Vertex3f(1.0f, -1.0f, 0.0f);
	glass4Batch.Vertex3f(-1.0f, -0.5f, 0.0f);
	glass4Batch.end();

	glGenTextures(2, textures);
	glBindTexture(GL_TEXTURE_2D, textures[0]);
	gltLoadTextureBMP("marble.bmp", GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR, GL_REPEAT);
	glBindTexture(GL_TEXTURE_2D, textures[1]);
	gltLoadTextureBMP("start_line.bmp", GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR, GL_REPEAT);

	// create and bind an FBO
	glGenFramebuffers(1, &msFBO);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, msFBO);

	// create depth texture
	glGenTextures(1, &depthTextureName);
	glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, depthTextureName);
	glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, 8, GL_DEPTH_COMPONENT24, screenWidth, screenHeight, GL_FALSE);

	// setup HDR render texture
	glGenTextures(1, msTexture);
	glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, msTexture[0]);
	glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, 8, GL_RGBA8, screenWidth, screenHeight, GL_FALSE);

	// create and bind FBO
	glGenFramebuffers(1, &msFBO);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, msFBO);

	// attach texture to first color attachment and depth RBO
	glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, msTexture[0], 0);
	glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D_MULTISAMPLE, depthTextureName, 0);

	// reset framebuffer binding
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

	// Load oit resolve shader
	oitResolve = gltLoadShaderWithFileEx("basic.vs", "oitResolve.fs", 3,
		GLT_ATTRIBUTE_VERTEX, "vVertex",
		GLT_ATTRIBUTE_NORMAL, "vNormal",
		GLT_ATTRIBUTE_TEXTURE0, "vTexCoord0");
	glBindFragDataLocation(oitResolve, 0, "oColor");
	glLinkProgram(oitResolve);

	// Load multisample resolve shader
	msResolve = gltLoadShaderWithFileEx("basic.vs", "msResolve.fs", 3,
		GLT_ATTRIBUTE_VERTEX, "vVertex",
		GLT_ATTRIBUTE_NORMAL, "vNormal",
		GLT_ATTRIBUTE_TEXTURE0, "vTexCoord0");

	glBindFragDataLocation(msResolve, 0, "oColor");
	glLinkProgram(msResolve);

	// Make sure all went well
	gltCheckErrors(oitResolve);
	gltCheckErrors(msResolve);

	int numMasks = 0;
	glGetIntegerv(GL_MAX_SAMPLE_MASK_WORDS, &numMasks);
	log("GL_MAX_SAMPLE_MASK_WORDS: %d", numMasks);
}

/**
 * Respond to arrow keys by moving the camera frame of reference
 */
void SpecialKeys(int key, int x, int y)
{
	static CStopWatch cameraTimer;
	float fTime = cameraTimer.delta();
	cameraTimer.reset();

	float linear = fTime * 3.0f;
	float angular = fTime * float(m3dDegToRad(60.0f));

	if (key == GLUT_KEY_LEFT)
	{
		worldAngle += angular * 50;
		if (worldAngle > 360)
			worldAngle -= 360;
	}

	if (key == GLUT_KEY_RIGHT)
	{
		worldAngle -= angular * 50;
		if (worldAngle < 360)
			worldAngle += 360;
	}
}

/**
 * Respond to the key pressed
 */
void KeyPressFunc(unsigned char key, int x, int y)
{
	if (key == 'o' || key == 'O')
		mode = USER_OIT;
	if (key == 'b' || key == 'B')
		mode = USER_BLEND;

	if (key == '1')
		blendMode = 1;
	if (key == '2')
		blendMode = 2;
	if (key == '3')
		blendMode = 3;
	if (key == '4')
		blendMode = 4;
	if (key == '5')
		blendMode = 5;
	if (key == '6')
		blendMode = 6;
	if (key == '7')
		blendMode = 7;

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
	// Bind the FBO with multisample buffers
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, msFBO);
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// User selected order independant transparency
	if (mode == USER_OIT)
	{
		// Use OIT, setup sample masks
		glSampleMaski(0, 0x01);
		glEnable(GL_SAMPLE_MASK);

		// Prevent depth test from culling covered surfaces
		glDepthFunc(GL_ALWAYS);
	}

	modelViewMatrix.push();
	M3DMatrix44f mCamera;
	cameraFrame.GetCameraMatrix(mCamera);
	modelViewMatrix.MultMatrix(mCamera);

	modelViewMatrix.push();
	modelViewMatrix.moveTo(0.0f, -0.4f, -4.0f);
	modelViewMatrix.rotateTo(worldAngle, 0.0, 1.0, 0.0);

	// Draw the background and disk to the first sample
	modelViewMatrix.push();
	modelViewMatrix.moveTo(0.0f, 3.0f, 0.0f);
	modelViewMatrix.rotateTo(90.0, 1.0, 0.0, 0.0);
	modelViewMatrix.rotateTo(90.0, 0.0, 0.0, 1.0);
	glBindTexture(GL_TEXTURE_2D, textures[1]);
	shaderManager.useStockShader(GLT_SHADER_TEXTURE_REPLACE, transformPipeline.GetMVPMatrix(), 0);
	bckgrndCylBatch.draw();
	modelViewMatrix.pop();

	modelViewMatrix.moveTo(0.0f, -0.3f, 0.0f);
	modelViewMatrix.push();
	modelViewMatrix.rotateTo(90.0, 1.0, 0.0, 0.0);
	shaderManager.useStockShader(GLT_SHADER_FLAT, transformPipeline.GetMVPMatrix(), vGrey);
	diskBatch.draw();
	modelViewMatrix.pop();
	modelViewMatrix.moveTo(0.0f, 0.1f, 0.0f);

	// User selected blending
	if (mode == USER_BLEND)
	{
		// Setup blend state
		glDisable(GL_DEPTH_TEST);
		glEnable(GL_BLEND);
		switch (blendMode)
		{
		case 1:
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			break;
		case 2:
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_DST_ALPHA);
			break;
		case 3:
			glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
			break;
		case 4:
			glBlendFunc(GL_SRC_ALPHA, GL_ONE);
			break;
		case 5:
			glBlendFunc(GL_SRC_ALPHA, GL_DST_COLOR);
			break;
		case 6:
			glBlendFuncSeparate(GL_SRC_ALPHA, GL_DST_ALPHA, GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			break;
		case 7:
			glBlendFuncSeparate(GL_SRC_COLOR, GL_DST_COLOR, GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			break;
		default:
			glDisable(GL_BLEND);
		}
	}

	// Now draw the glass pieces
	DrawWorld();

	modelViewMatrix.pop();
	modelViewMatrix.pop();

	// Clean up all state 
	glDepthFunc(GL_LEQUAL);
	glDisable(GL_BLEND);
	glDisable(GL_SAMPLE_MASK);
	glSampleMaski(0, 0xffffffff);

	// Resolve multisample buffer
	projectionMatrix.push();
	projectionMatrix.setMatrix(orthoMatrix);
	modelViewMatrix.push();
	modelViewMatrix.identity();
	// Setup and Clear the default framebuffer
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
	glViewport(0, 0, screenWidth, screenHeight);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	if (mode == USER_OIT)
		SetupOITResolveProg();
	else if (mode == USER_BLEND)
		SetupResolveProg();

	// Draw a full-size quad to resolve the multisample surfaces
	screenQuad.draw();
	modelViewMatrix.pop();
	projectionMatrix.pop();

	// Reset texture state
	glEnable(GL_DEPTH_TEST);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);

	// Perform the buffer swap to display back buffer
	glutSwapBuffers();
	glutPostRedisplay();
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
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, 0);

	glDeleteTextures(1, msTexture);
	glDeleteTextures(1, &depthTextureName);
	glDeleteTextures(1, textures);

	// Cleanup FBOs
	glDeleteFramebuffers(1, &msFBO);
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
