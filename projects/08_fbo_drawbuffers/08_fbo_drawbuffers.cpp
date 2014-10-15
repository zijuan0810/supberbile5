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
bool                isUseFBO;
GLuint              fboBuffer;
GLuint              depthBufferName;
GLuint				renderBufferNames[3];

SBObject            ninja;
GLuint              ninjaTex[1];

static void draw_world(GLfloat yRot);
static void use_process_program(M3DVector4f vLightPos, M3DVector4f vColor, int textureUnit);

static float* load_float_data(const char *szFile, int *count)
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

// Enable and setup the GLSL program used for 
// flushes, etc.
static void use_process_program(M3DVector4f vLightPos, M3DVector4f vColor, int textureUnit)
{
	glUseProgram(processProg);

	// set Matricies for Vertex Program
	glUniformMatrix4fv(glGetUniformLocation(processProg, "mvMatrix"), 1, GL_FALSE,
		transformPipeline.GetModelViewMatrix());
	glUniformMatrix4fv(glGetUniformLocation(processProg, "pMatrix"), 1, GL_FALSE,
		transformPipeline.GetProjectionMatrix());

	// set the light position
	glUniform3fv(glGetUniformLocation(processProg, "vLightPos"), 1, vLightPos);
	// set the vertex color for rendered pixels
	glUniform4fv(glGetUniformLocation(processProg, "vColor"), 1, vColor);
	// set the texture unit for the texBO fetch
	glUniform1i(glGetUniformLocation(processProg, "lumCurveSampler"), 1);

	// if this geometry is textured, set the texture unit
	if (textureUnit != -1) {
		glUniform1i(glGetUniformLocation(processProg, "bUseTexture"), 1);
		glUniform1i(glGetUniformLocation(processProg, "textureUnit0"), textureUnit);
	}
	else {
		glUniform1i(glGetUniformLocation(processProg, "bUseTexture"), 0);
	}

	gltCheckErrors();
}

// Draw the scene
static void draw_world(GLfloat yRot)
{
	M3DMatrix44f mCamera;
	modelViewMatrix.GetMatrix(mCamera);

	// need light position relative to the Camera
	M3DVector4f vLightTransformed;
	m3dTransformVector4(vLightTransformed, vLightPos, mCamera);

	// Draw the light source as a small white unshaded sphere
	modelViewMatrix.push();
	modelViewMatrix.moveTo(vLightPos);
	if (isUseFBO) {
		use_process_program(vLightPos, vWhite, -1);
	}
	else {
		shaderManager.useStockShader(GLT_SHADER_FLAT, transformPipeline.GetMVPMatrix(), vWhite);
	}
	sphereBatch.draw();
	modelViewMatrix.pop();

	// Draw stuff relative to the camera
	modelViewMatrix.push();
	modelViewMatrix.moveTo(0.0f, 0.2f, -2.5f);
		modelViewMatrix.push();
		modelViewMatrix.rotateTo(yRot, 0.0f, 1.0f, 0.0f);
		modelViewMatrix.moveTo(0.0f, (GLfloat)-0.6f, 0.0);
		modelViewMatrix.scaleTo((GLfloat)0.02, (GLfloat)0.006, (GLfloat)0.02);

		glBindTexture(GL_TEXTURE_2D, ninjaTex[0]);
		if (isUseFBO) {
			use_process_program(vLightTransformed, vWhite, 0);
		}
		else {
			shaderManager.useStockShader(GLT_SHADER_TEXTURE_REPLACE, transformPipeline.GetMVPMatrix(), 0);
		}
		ninja.Render(0, 0);

		modelViewMatrix.pop();
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

	// update screen sizes
	screenWidth = w;
	screenHeight = h;

	glBindRenderbuffer(GL_RENDERBUFFER, depthBufferName);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT32, w, h);

	for (int i = 0; i < 3; ++i) {
		glBindRenderbuffer(GL_RENDERBUFFER, renderBufferNames[i]);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA8, w, h);
	}
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

	glGenTextures(1, ninjaTex);
	glBindTexture(GL_TEXTURE_2D, ninjaTex[0]);
	gltLoadTextureBMP("NinjaComp.bmp", GL_LINEAR, GL_LINEAR, GL_CLAMP);

	// 查询RBO支持的最大内存空间，即glRenderbufferStorage分配的空间宽度、高度必须小于该值
	GLint maxRenderbufferSize;
	glGetIntegerv(GL_MAX_RENDERBUFFER_SIZE, &maxRenderbufferSize);
	log("GL_MAX_RENDERBUFFER_SIZE: %d", maxRenderbufferSize);

	// 创建帧缓存区
	glGenFramebuffers(1, &fboBuffer);

	// 创建RBO
	// Create depth renderbuffer
	glGenRenderbuffers(1, &depthBufferName);
	glBindRenderbuffer(GL_RENDERBUFFER, depthBufferName); // RBO只能绑定到GL_RENDERBUFFER目标上
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT32, screenWidth, screenHeight); // 分配内存空间

	// Create 3 color renderbuffers
	glGenRenderbuffers(3, renderBufferNames);
	for (int i = 0; i < 3; ++i) {
		glBindRenderbuffer(GL_RENDERBUFFER, renderBufferNames[i]);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA8, screenWidth, screenHeight);
	}

	// 查询GL允许一次最多绑定多少个颜色缓冲区
	GLint maxColorAttachments = 0;
	glGetIntegerv(GL_MAX_COLOR_ATTACHMENTS, &maxColorAttachments);
	log("GL_MAX_COLOR_ATTACHMENTS: %d", maxColorAttachments);

	// Attach all 4 renderbuffers to FBO
	// 将FBO与RBO链接起来
	// 一个帧缓冲区（FBO）可以有多个绑定点：一个深度绑定点，一个模版绑定点和多个颜色绑定点
	glBindRenderbuffer(GL_DRAW_FRAMEBUFFER, fboBuffer);
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
	glLinkProgram(processProg);

	CHECK_GL_ERROR();

	// Create 3 new buffer objects
	glGenBuffers(3, texBO);
	glGenTextures(1, &texBOTexture);

	// Load first texBO with a tangent-like curve, 1024 values
	int count = 0;
	//float* fileData = LoadFloatData("LumTan.data", &count);
	shared_ptr<float> fileData(load_float_data("LumTan.data", &count));
	if (count > 0) {
		glBindBuffer(GL_TEXTURE_BUFFER_ARB, texBO[0]);
		glBufferData(GL_TEXTURE_BUFFER_ARB, count * sizeof(float), (float*)(fileData.get()), GL_STATIC_DRAW);
	}

	// Load second texBO with a sine-like curve, 1024 values
	fileData = shared_ptr<float>(load_float_data("LumSin.data", &count));
	if (count > 0) {
		glBindBuffer(GL_TEXTURE_BUFFER_ARB, texBO[1]);
		glBufferData(GL_TEXTURE_BUFFER_ARB, count * sizeof(float), (float*)(fileData.get()), GL_STATIC_DRAW);
	}

	// Load third texBO with a linear curve, 1024 values
	fileData = shared_ptr<float>(load_float_data("LumLinear.data", &count));
	if (count > 0) {
		glBindBuffer(GL_TEXTURE_BUFFER_ARB, texBO[2]);
		glBufferData(GL_TEXTURE_BUFFER_ARB, count * sizeof(float), (float*)(fileData.get()), GL_STATIC_DRAW);
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
	static CStopWatch cameraTimer;
	float fTime = cameraTimer.delta();
	cameraTimer.Reset();

	float linear = fTime * 3.0f;
	float angular = fTime * float(m3dDegToRad(60.0f));

	if (key == GLUT_KEY_UP)
		cameraFrame.MoveForward(linear);

	if (key == GLUT_KEY_DOWN)
		cameraFrame.MoveForward(-linear);

	if (key == GLUT_KEY_LEFT)
		cameraFrame.RotateWorld(angular, 0.0f, 1.0f, 0.0f);

	if (key == GLUT_KEY_RIGHT)
		cameraFrame.RotateWorld(-angular, 0.0f, 1.0f, 0.0f);

	static bool bF2IsDown = false;
	if (key == GLUT_KEY_F2) {
		if (bF2IsDown == false) {
			bF2IsDown = true;
			isUseFBO = !isUseFBO;
		}
	}
	else {
		bF2IsDown = false;
	}

	if (key == GLUT_KEY_F3) {
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_BUFFER_ARB, texBOTexture);
		glTexBufferARB(GL_TEXTURE_BUFFER_ARB, GL_R32F, texBO[0]); // FIX this in glee
		glActiveTexture(GL_TEXTURE0);
	}
	else if (key == GLUT_KEY_F4) {
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_BUFFER_ARB, texBOTexture);
		glTexBufferARB(GL_TEXTURE_BUFFER_ARB, GL_R32F, texBO[1]);
		glActiveTexture(GL_TEXTURE0);
	}
	else if (key == GLUT_KEY_F5) {
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_BUFFER_ARB, texBOTexture);
		glTexBufferARB(GL_TEXTURE_BUFFER_ARB, GL_R32F, texBO[2]);
		glActiveTexture(GL_TEXTURE0);
	}

	glutPostRedisplay();
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
	float yRot = animationTimer.delta() * 60.0f;

	modelViewMatrix.push();
	
	M3DMatrix44f mCamera;
	cameraFrame.GetCameraMatrix(mCamera);
	modelViewMatrix *= mCamera;

	GLfloat vFloorColor[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
	if (isUseFBO) {
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fboBuffer);
		glDrawBuffers(3, fboBuffs); // 自定义着色器输入路由
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// need light position relative to the Camera
		M3DVector4f vLightTransformed;
		m3dTransformVector4(vLightTransformed, vLightPos, mCamera);
		use_process_program(vLightTransformed, vFloorColor, 0);
	}
	else {
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
		glDrawBuffers(1, windowBuff);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		shaderManager.useStockShader(GLT_SHADER_TEXTURE_MODULATE, transformPipeline.GetMVPMatrix(), vFloorColor, 0);
	}

	glBindTexture(GL_TEXTURE_2D, textures[0]); // Marble
	floorBatch.draw();
	draw_world(yRot);

	modelViewMatrix.pop();

	if (isUseFBO) {
		// Direct drawing to the window
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
		glDrawBuffers(1, windowBuff);
		glViewport(0, 0, screenWidth, screenHeight);

		// Source buffer reads from the framebuffer object
		glBindFramebuffer(GL_READ_FRAMEBUFFER, fboBuffer);

		// Copy greyscale output to the left half of the screen
		glReadBuffer(GL_COLOR_ATTACHMENT1); // 指定glBlitFramebuffer读取的缓冲区
		glBlitFramebuffer(0, 0, screenWidth / 2, screenHeight, 0, 0, screenWidth / 2, screenHeight,
			GL_COLOR_BUFFER_BIT, GL_NEAREST);

		// Copy the luminance adjusted color to the right half of the screen
		glReadBuffer(GL_COLOR_ATTACHMENT2);
		glBlitFramebuffer(screenWidth / 2, 0, screenWidth, screenHeight,
			screenWidth / 2, 0, screenWidth, screenHeight,
			GL_COLOR_BUFFER_BIT, GL_NEAREST);

		// Scale the unaltered image to the upper right of the screen
		glReadBuffer(GL_COLOR_ATTACHMENT0);
		glBlitFramebuffer(0, 0, screenWidth, screenHeight,
			(int)(screenWidth *(0.8)), (int)(screenHeight*(0.8)), screenWidth, screenHeight,
			GL_COLOR_BUFFER_BIT, GL_LINEAR);

		glBindTexture(GL_TEXTURE_2D, 0);
	}

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
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_BUFFER_ARB, 0);
	glActiveTexture(GL_TEXTURE0);

	glDeleteTextures(1, &texBOTexture);
	glDeleteTextures(1, textures);
	glDeleteTextures(1, ninjaTex);

	// Cleanup RBOs
	glDeleteRenderbuffers(3, renderBufferNames);
	glDeleteRenderbuffers(1, &depthBufferName);

	// cleanup FBOs
	glDeleteFramebuffers(1, &fboBuffer);

	// cleanup Buffer objects
	glDeleteBuffers(3, texBO);

	// cleanup programs
	glUseProgram(0);
	glDeleteProgram(processProg);

	ninja.Free();
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
