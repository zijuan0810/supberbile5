#include <GLTools.h>
#include <GL/glut.h>
#include <GLShaderManager.h>

#include <GLFrustum.h>
#include <GLBatch.h>
#include <GLMatrixStack.h>
#include <GLGeometryTransform.h>
#include <StopWatch.h>

#include <math.h>
#include <stdio.h>

#include <iostream>

GLShaderManager	shaderManager;

GLMatrixStack		modelViewMarix;
GLMatrixStack		projectionMatrix;

GLFrame		cameraFrame;
GLFrame		objectFrame;
GLFrustum	viewFrustum;
GLBatch		pyramidBatch;

GLuint		texturedId;

GLGeometryTransform	transformPipeline;
M3DMatrix44f		shadowMarix;

GLTriangleBatch		torusBatch;
GLTriangleBatch		sphereBatch;
GLBatch				cubeBatch;

void _createPyramid(GLBatch& outBatch)
{
	outBatch.Begin(GL_TRIANGLES, 18, 1);

	// Bottom of pyramid
	outBatch.Normal3f(0.0f, -1.0f, 0.0f);
	outBatch.MultiTexCoord2f(0, 0.0f, 0.0f);
	outBatch.Vertex3f(-1.0f, -1.0f, -1.0f);

	outBatch.Normal3f(0.0f, -1.0f, 0.0f);
	outBatch.MultiTexCoord2f(0, 1.0f, 0.0f);
	outBatch.Vertex3f(1.0f, -1.0f, -1.0f);

	outBatch.Normal3f(0.0f, -1.0f, 0.0f);
	outBatch.MultiTexCoord2f(0, 1.0f, 1.0f);
	outBatch.Vertex3f(1.0f, -1.0f, 1.0f);

	outBatch.Normal3f(0.0f, -1.0f, 0.0f);
	outBatch.MultiTexCoord2f(0, 0.0f, 1.0f);
	outBatch.Vertex3f(-1.0f, -1.0f, 1.0f);

	outBatch.Normal3f(0.0f, -1.0f, 0.0f);
	outBatch.MultiTexCoord2f(0, 0.0f, 0.0f);
	outBatch.Vertex3f(-1.0f, -1.0f, -1.0f);

	outBatch.Normal3f(0.0f, -1.0f, 0.0f);
	outBatch.MultiTexCoord2f(0, 1.0f, 1.0f);
	outBatch.Vertex3f(1.0f, -1.0f, 1.0f);


	M3DVector3f vApex = { 0.0f, 1.0f, 0.0f };
	M3DVector3f vFrontLeft = { -1.0f, -1.0f, 1.0f };
	M3DVector3f vFrontRight = { 1.0f, -1.0f, 1.0f };
	M3DVector3f vBackLeft = { -1.0f, -1.0f, -1.0f };
	M3DVector3f vBackRight = { 1.0f, -1.0f, -1.0f };
	M3DVector3f n;

	// Front of Pyramid
	m3dFindNormal(n, vApex, vFrontLeft, vFrontRight);
	outBatch.Normal3fv(n);
	outBatch.MultiTexCoord2f(0, 0.5f, 1.0f);
	outBatch.Vertex3fv(vApex);		// Apex

	outBatch.Normal3fv(n);
	outBatch.MultiTexCoord2f(0, 0.0f, 0.0f);
	outBatch.Vertex3fv(vFrontLeft);		// Front left corner

	outBatch.Normal3fv(n);
	outBatch.MultiTexCoord2f(0, 1.0f, 0.0f);
	outBatch.Vertex3fv(vFrontRight);		// Front right corner


	m3dFindNormal(n, vApex, vBackLeft, vFrontLeft);
	outBatch.Normal3fv(n);
	outBatch.MultiTexCoord2f(0, 0.5f, 1.0f);
	outBatch.Vertex3fv(vApex);		// Apex

	outBatch.Normal3fv(n);
	outBatch.MultiTexCoord2f(0, 1.0f, 0.0f);
	outBatch.Vertex3fv(vBackLeft);		// Back left corner

	outBatch.Normal3fv(n);
	outBatch.MultiTexCoord2f(0, 0.0f, 0.0f);
	outBatch.Vertex3fv(vFrontLeft);		// Front left corner

	m3dFindNormal(n, vApex, vFrontRight, vBackRight);
	outBatch.Normal3fv(n);
	outBatch.MultiTexCoord2f(0, 0.5f, 1.0f);
	outBatch.Vertex3fv(vApex);				// Apex

	outBatch.Normal3fv(n);
	outBatch.MultiTexCoord2f(0, 1.0f, 0.0f);
	outBatch.Vertex3fv(vFrontRight);		// Front right corner

	outBatch.Normal3fv(n);
	outBatch.MultiTexCoord2f(0, 0.0f, 0.0f);
	outBatch.Vertex3fv(vBackRight);			// Back right cornder


	m3dFindNormal(n, vApex, vBackRight, vBackLeft);
	outBatch.Normal3fv(n);
	outBatch.MultiTexCoord2f(0, 0.5f, 1.0f);
	outBatch.Vertex3fv(vApex);		// Apex

	outBatch.Normal3fv(n);
	outBatch.MultiTexCoord2f(0, 0.0f, 0.0f);
	outBatch.Vertex3fv(vBackRight);		// Back right cornder

	outBatch.Normal3fv(n);
	outBatch.MultiTexCoord2f(0, 1.0f, 0.0f);
	outBatch.Vertex3fv(vBackLeft);		// Back left corner

	outBatch.End();
}

/**
 * Load a TGA as a 2D texture. Completely initialize the state
 */
bool _loadTgaTexture(const char* szFileName, GLenum minFilter, GLenum magFilter, GLenum wrapMode)
{
	int nWidth, nHeight, nComponents;
	GLenum eFormat;
	// Read the texture bits
	GLbyte* pBits = gltReadTGABits(szFileName, &nWidth, &nHeight, &nComponents, &eFormat);
	if ( pBits == nullptr ) {
		return false;
	}

	// Set texture warp mode
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapMode);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapMode);

	// Set texture filter mode
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minFilter);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, magFilter);

	// 设置纹理存储格式
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	// 加载纹理
	glTexImage2D(GL_TEXTURE_2D, 0, nComponents, nWidth, nHeight, 0, eFormat, GL_UNSIGNED_BYTE, 
				pBits);
	free(pBits);

	if ( minFilter == GL_LINEAR_MIPMAP_LINEAR || minFilter == GL_LINEAR_MIPMAP_NEAREST ||
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
	glViewport(0, 0, w, h);
	viewFrustum.SetPerspective(35.0f, (float)w/(float)h, 1.0f, 500.f);
	projectionMatrix.setMatrix(viewFrustum.GetProjectionMatrix());
	transformPipeline.SetMatrixStacks(modelViewMarix, projectionMatrix);
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

	// 产生n个未用的纹理对象标识符，将标识符存在textures这个数组中
	glGenTextures(1, &texturedId);
	// 绑定纹理对象
	glBindTexture(GL_TEXTURE_2D, texturedId);
	// 从文件中加载纹理
	//_loadTgaTexture("stone.tga", GL_LINEAR, GL_LINEAR, GL_CLAMP_TO_EDGE);
	//_loadTgaTexture("stone.tga", GL_NEAREST, GL_NEAREST, GL_CLAMP_TO_BORDER);
	_loadTgaTexture("bang.tga", GL_NEAREST, GL_LINEAR, GL_CLAMP_TO_EDGE);
	// 创建金字塔
	_createPyramid(pyramidBatch);


	// This make torus
	gltMakeTorus(torusBatch, 0.4f, 0.15f, 30, 30);
	// This make sphere
	gltMakeSphere(sphereBatch, 0.2f, 30, 30);
	// This make cube
	gltMakeCube(cubeBatch, 1.0f);

	cameraFrame.MoveForward(-7.0f);
}

/**
 * Respond to arrow keys by moving the camera frame of reference
 */
void SpecialKeys(int key, int x, int y)
{
	if(key == GLUT_KEY_UP)
		objectFrame.RotateWorld(m3dDegToRad(-5.0f), 1.0f, 0.0f, 0.0f);

	if(key == GLUT_KEY_DOWN)
		objectFrame.RotateWorld(m3dDegToRad(5.0f), 1.0f, 0.0f, 0.0f);

	if(key == GLUT_KEY_LEFT)
		objectFrame.RotateWorld(m3dDegToRad(-5.0f), 0.0f, 1.0f, 0.0f);

	if(key == GLUT_KEY_RIGHT)
		objectFrame.RotateWorld(m3dDegToRad(5.0f), 0.0f, 1.0f, 0.0f);

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
	static GLfloat vLightPos[] = { 1.0f, 1.0f, 0.0f };
	static GLfloat vWhite[] = { 1.0f, 1.0f, 1.0f, 1.0f };

	// Clear the window with current clearing color
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	modelViewMarix.push();
		M3DMatrix44f mCamera;
		cameraFrame.GetCameraMatrix(mCamera);
		modelViewMarix.MultMatrix(mCamera);

		M3DMatrix44f mObjectFrame;
		objectFrame.GetMatrix(mObjectFrame);
		modelViewMarix.MultMatrix(mObjectFrame);

		glBindTexture(GL_TEXTURE_2D, texturedId);
		shaderManager.useStockShader(GLT_SHADER_TEXTURE_POINT_LIGHT_DIFF,
									transformPipeline.GetModelViewMatrix(),
									transformPipeline.GetProjectionMatrix(),
									vLightPos, vWhite, 0);
		//pyramidBatch.draw();
		//torusBatch.draw();
		cubeBatch.draw();
	modelViewMarix.pop();

	if ( gltIsExtSupported("GL_EXT_texture_filter_anisotropic") ) {
		std::cout << "suppter";
	}

	// Perform the buffer swap to display back buffer
	glutSwapBuffers();
}

/**
 * Cleanup... such as deleting texture objects
 */
void OnExit()
{
	glDeleteTextures(1, &texturedId);
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

	gltPrintOpenGLInfo();

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
