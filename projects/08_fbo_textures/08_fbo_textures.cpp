#include "GLTool-ext.h"

static GLfloat vGreen[] = { 0.0f, 1.0f, 0.0f, 1.0f };
static GLfloat vBlue[] = { 0.0f, 0.0f, 1.0f, 1.0f };
static GLfloat vWhite[] = { 1.0f, 1.0f, 1.0f, 1.0f };
static GLfloat vBlack[] = { 0.0f, 0.0f, 0.0f, 1.0f };
static GLfloat vGrey[] = { 0.5f, 0.5f, 0.5f, 1.0f };
static GLfloat vLightPos[] = { -2.0f, 3.0f, -2.0f, 1.0f };
static const GLenum windowBuff[] = { GL_BACK_LEFT };
static const GLenum fboBuffs[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
static GLint mirrorTexWidth = 800;
static GLint mirrorTexHeight = 800;

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
GLFrame				mirrorFrame;			// Mirror frame

GLTriangleBatch		torusBatch;
GLTriangleBatch		sphereBatch;
GLTriangleBatch		cylinderBatch;
GLBatch				floorBatch;
GLBatch				mirrorBatch;
GLBatch				mirrorBorderBatch;

GLuint              fboName;
GLuint				textures[1];
GLuint				mirrorTexture;
GLuint              depthBufferName;

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
		shaderManager.useStockShader(GLT_SHADER_FLAT, transformPipeline.GetMVPMatrix(), vWhite);
		sphereBatch.draw();
	modelViewMatrix.pop();

	// Draw stuff relative to the camera
	modelViewMatrix.push();
		modelViewMatrix.moveTo(0.0f, 0.2f, -2.5f);
		modelViewMatrix.rotateTo(yRot, 0.0f, 1.0f, 0.0f);
		shaderManager.useStockShader(GLT_SHADER_POINT_LIGHT_DIFF, modelViewMatrix.GetMatrix(),
			transformPipeline.GetProjectionMatrix(), vLightTransformed, vGreen, 0);
		torusBatch.draw();
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

	// Black
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

	gltCreateTorus(torusBatch, 0.4f, 0.15f, 35, 35);
	gltCreateSphere(sphereBatch, 0.1f, 26, 13);
	gltCreateCylinder(cylinderBatch, 0.3f, 0.2f, 1.0, 10, 10);

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

	mirrorBatch.begin(GL_TRIANGLE_FAN, 4, 1);
	mirrorBatch.Color4f(1.0f, 0.0f, 0.0f, 1.0f);
	mirrorBatch.MultiTexCoord2f(0, 0.0f, 0.0f);
	mirrorBatch.Normal3f(0.0f, 1.0f, 0.0f);
	mirrorBatch.Vertex3f(-1.0f, 0.0f, 0.0f);

	mirrorBatch.Color4f(1.0f, 0.0f, 0.0f, 1.0f);
	mirrorBatch.MultiTexCoord2f(0, 1.0f, 0.0f);
	mirrorBatch.Normal3f(0.0f, 1.0f, 0.0f);
	mirrorBatch.Vertex3f(1.0f, 0.0f, 0.0f);

	mirrorBatch.Color4f(1.0f, 0.0f, 0.0f, 1.0f);
	mirrorBatch.MultiTexCoord2f(0, 1.0f, 1.0f);
	mirrorBatch.Normal3f(0.0f, 1.0f, 0.0f);
	mirrorBatch.Vertex3f(1.0f, 2.0f, 0.0f);

	mirrorBatch.Color4f(1.0f, 0.0f, 0.0f, 1.0f);
	mirrorBatch.MultiTexCoord2f(0, 0.0f, 1.0f);
	mirrorBatch.Normal3f(0.0f, 1.0f, 0.0f);
	mirrorBatch.Vertex3f(-1.0f, 2.0f, 0.0f);
	mirrorBatch.end();

	mirrorBorderBatch.begin(GL_TRIANGLE_STRIP, 13);
	mirrorBorderBatch.Normal3f(0.0f, 0.0f, 1.0f);
	mirrorBorderBatch.Vertex3f(-1.0f, 0.1f, 0.01f);

	mirrorBorderBatch.Normal3f(0.0f, 0.0f, 1.0f);
	mirrorBorderBatch.Vertex3f(-1.0f, 0.0f, 0.01f);

	mirrorBorderBatch.Normal3f(0.0f, 0.0f, 1.0f);
	mirrorBorderBatch.Vertex3f(1.0f, 0.1f, 0.01f);

	mirrorBorderBatch.Normal3f(0.0f, 0.0f, 1.0f);
	mirrorBorderBatch.Vertex3f(1.0f, 0.0f, 0.01f);

	mirrorBorderBatch.Normal3f(0.0f, 0.0f, 1.0f);
	mirrorBorderBatch.Vertex3f(0.9f, 0.0f, 0.01f);

	mirrorBorderBatch.Normal3f(0.0f, 0.0f, 1.0f);
	mirrorBorderBatch.Vertex3f(1.0f, 2.0f, 0.01f);

	mirrorBorderBatch.Normal3f(0.0f, 0.0f, 1.0f);
	mirrorBorderBatch.Vertex3f(0.9f, 2.0f, 0.01f);

	mirrorBorderBatch.Normal3f(0.0f, 0.0f, 1.0f);
	mirrorBorderBatch.Vertex3f(1.0f, 1.9f, 0.01f);

	mirrorBorderBatch.Normal3f(0.0f, 0.0f, 1.0f);
	mirrorBorderBatch.Vertex3f(-1.0f, 2.f, 0.01f);

	mirrorBorderBatch.Normal3f(0.0f, 0.0f, 1.0f);
	mirrorBorderBatch.Vertex3f(-1.0f, 1.9f, 0.01f);

	mirrorBorderBatch.Normal3f(0.0f, 0.0f, 1.0f);
	mirrorBorderBatch.Vertex3f(-0.9f, 2.f, 0.01f);

	mirrorBorderBatch.Normal3f(0.0f, 0.0f, 1.0f);
	mirrorBorderBatch.Vertex3f(-1.0f, 0.0f, 0.01f);

	mirrorBorderBatch.Normal3f(0.0f, 0.0f, 1.0f);
	mirrorBorderBatch.Vertex3f(-0.9f, 0.0f, 0.01f);
	mirrorBorderBatch.end();

	glGenTextures(1, textures);
	glBindTexture(GL_TEXTURE_2D, textures[0]);
	gltLoadTextureBMP("marble.bmp", GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR, GL_REPEAT);

	// create and bind an FBO
	glGenFramebuffers(1, &fboName);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fboName);

	// create depth renderbuffer
	glGenRenderbuffers(1, &depthBufferName);
	glBindRenderbuffer(GL_RENDERBUFFER, depthBufferName);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT32, mirrorTexWidth, mirrorTexHeight);

	// create the reflection texture
	glGenTextures(1, &mirrorTexture);
	glBindTexture(GL_TEXTURE_2D, mirrorTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, mirrorTexWidth, mirrorTexHeight, 0, GL_RGBA, GL_FLOAT, nullptr);

	// attach texture to first color attachment and the depth RBO
	glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mirrorTexture, 0);
	glFramebufferRenderbuffer(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthBufferName);

	CHECK_GL_ERROR();

	// reset framebuffer binding
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
}

/**
 * Respond to arrow keys by moving the camera frame of reference
 */
void SpecialKeys(int key, int x, int y)
{
	static CStopWatch cameraTimer;
	float fTime = cameraTimer.delta();
	cameraTimer.reset();

	float liner = fTime * 0.6f;
	float angular = fTime * float(m3dDegToRad(60.0f));

	if (key == GLUT_KEY_UP) {
		cameraFrame.MoveForward(liner);
	}
	else if (key == GLUT_KEY_DOWN) {
		cameraFrame.MoveForward(-liner);
	}
	else if (key == GLUT_KEY_LEFT) {
		cameraFrame.RotateWorld(angular, 0.0f, 1.0f, 0.0f);
	}
	else if (key == GLUT_KEY_RIGHT) {
		cameraFrame.RotateWorld(-angular, 0.0f, 1.0f, 0.0f);
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
	float yRot = animationTimer.delta() * 60.0f;

	M3DVector3f vCameraPos;
	M3DVector3f vCameraForward;
	M3DVector3f vMirrorPos;
	M3DVector3f vMirrorForward;
	cameraFrame.GetOrigin(vCameraPos);
	cameraFrame.GetForwardVector(vCameraForward);

	// Set position of mirror frame (camera)
	vMirrorPos[0] = 0.0;
	vMirrorPos[1] = 0.1f;
	vMirrorPos[2] = -6.0f; // view pos is actually behind mirror
	mirrorFrame.SetOrigin(vMirrorPos);

	// Calculate direction of mirror frame (camera)
	// Because the position of the mirror is known relative to the origin
	// find the direction vector by adding the mirror offset to the vector
	// of the viewer-origin
	vMirrorForward[0] = vCameraPos[0];
	vMirrorForward[1] = vCameraPos[1];
	vMirrorForward[2] = (vCameraPos[2] + 5);
	m3dNormalizeVector3(vMirrorForward);
	mirrorFrame.SetForwardVector(vMirrorForward);

	// first render from the mirrors perspective
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fboName);
	glDrawBuffers(1, fboBuffs);
	glViewport(0, 0, mirrorTexWidth, mirrorTexHeight);


	// Draw scene from the perspective of the mirror camera
	modelViewMatrix.push();

	M3DMatrix44f mMirrorView;
	mirrorFrame.GetCameraMatrix(mMirrorView);
	modelViewMatrix.MultMatrix(mMirrorView);

	// Flip the mirror camera horizontally for the reflection
	modelViewMatrix.scaleTo(-1.0f, 1.0f, 1.0f);

	glBindTexture(GL_TEXTURE_2D, textures[0]); // Marble
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	shaderManager.useStockShader(GLT_SHADER_TEXTURE_MODULATE, transformPipeline.GetMVPMatrix(), vWhite, 0);
	floorBatch.draw();
	draw_world(yRot);

	// Now draw a cylinder representing the viewer
	M3DVector4f vLightTransformed;
	modelViewMatrix.GetMatrix(mMirrorView);
	m3dTransformVector4(vLightTransformed, vLightPos, mMirrorView);
	modelViewMatrix.moveTo(vCameraPos[0], vCameraPos[1] - 0.8f, vCameraPos[2] - 1.0f);
	modelViewMatrix.rotateTo(-90.0f, 1.0f, 0.0f, 0.0f);

	shaderManager.useStockShader(GLT_SHADER_POINT_LIGHT_DIFF, modelViewMatrix.GetMatrix(),
		transformPipeline.GetProjectionMatrix(), vLightTransformed, vBlue, 0);
	cylinderBatch.draw();
	modelViewMatrix.pop();

	// Reset FBO. Draw world again from the real cameras perspective
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
	glDrawBuffers(1, windowBuff);
	glViewport(0, 0, screenWidth, screenHeight);

	modelViewMatrix.push();
	M3DMatrix44f mCamera;
	cameraFrame.GetCameraMatrix(mCamera);
	modelViewMatrix.MultMatrix(mCamera);

	glBindTexture(GL_TEXTURE_2D, textures[0]); // Marble
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	shaderManager.useStockShader(GLT_SHADER_TEXTURE_MODULATE, transformPipeline.GetMVPMatrix(), vWhite, 0);

	floorBatch.draw();
	draw_world(yRot);

	// Now draw the mirror surfaces
	modelViewMatrix.push();
	modelViewMatrix.moveTo(0.0f, -0.4f, -5.0f);
	if (vCameraPos[2] > -5.0) {
		glBindTexture(GL_TEXTURE_2D, mirrorTexture); // Reflection
		shaderManager.useStockShader(GLT_SHADER_TEXTURE_REPLACE, transformPipeline.GetMVPMatrix(), 0);
	}
	else {
		// If the camera is behind the mirror, just draw black
		shaderManager.useStockShader(GLT_SHADER_FLAT, transformPipeline.GetMVPMatrix(), vBlack);
	}
	mirrorBatch.draw();
	shaderManager.useStockShader(GLT_SHADER_FLAT, transformPipeline.GetMVPMatrix(), vGrey);
	mirrorBorderBatch.draw();

	modelViewMatrix.pop();
	modelViewMatrix.pop();

	// Perform the buffer swap to display back buffer
	glutSwapBuffers();
	glutPostRedisplay();
}

/**
 * Cleanup... such as deleting texture objects
 */
void OnExit()
{
	// make sure default FBO is bound
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
	glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);

	// cleanup textures
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, 0);

	glDeleteTextures(1, &mirrorTexture);
	glDeleteTextures(1, textures);

	// cleanup RBOs
	glDeleteRenderbuffers(1, &depthBufferName);

	// cleanup FBOs
	glDeleteFramebuffers(1, &fboName);
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
