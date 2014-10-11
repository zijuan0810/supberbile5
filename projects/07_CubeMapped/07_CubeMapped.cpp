#include "GLTool-ext.h"

GLShaderManager		shaderManager;
GLFrame             viewFrame;
GLFrustum           viewFrustum;
GLTriangleBatch     sphereBatch;
GLBatch             cubeBatch;
GLMatrixStack       modelViewMatrix;
GLMatrixStack       projectionMatrix;
GLGeometryTransform transformPipeline;
GLuint              cubeTexture;
GLint               reflectionShader;
GLint               skyBoxShader;

GLint	locMVPReflect, locMVReflect, locNormalReflect, locInvertedCamera;
GLint	locMVPSkyBox;

// Six sides of a cube map
const char *szCubeFaces[6] = { "pos_x.tga", "neg_x.tga", "pos_y.tga", "neg_y.tga", "pos_z.tga", "neg_z.tga" };

GLenum  cube_map[6] = { 
	GL_TEXTURE_CUBE_MAP_POSITIVE_X,
	GL_TEXTURE_CUBE_MAP_NEGATIVE_X,
	GL_TEXTURE_CUBE_MAP_POSITIVE_Y,
	GL_TEXTURE_CUBE_MAP_NEGATIVE_Y,
	GL_TEXTURE_CUBE_MAP_POSITIVE_Z,
	GL_TEXTURE_CUBE_MAP_NEGATIVE_Z 
};


/**
 * Window has changed size, or has just been created. In either case, we need to use the window 
 * dimensions to set the viewport and the projection matrix.
 */
void ChangeSize(int w, int h)
{
	if ( h == 0 ) {
		h = 1;
	}

	glViewport(0, 0, w, h);
	
	viewFrustum.SetPerspective(35.0f, float(w) / h, 1.0f, 1000.0f);

	projectionMatrix.LoadMatrix(viewFrustum.GetProjectionMatrix());
	transformPipeline.SetMatrixStacks(modelViewMatrix, projectionMatrix);
}

/**
 * This function does any needed initialization on the rendering context.
 * This is the first opportunity to do any OpenGL related tasks.
 */
void SetupRC()
{
	// Cull back of polygons
	glCullFace(GL_BACK);
	glFrontFace(GL_CCW);
	glEnable(GL_DEPTH_TEST);

	glGenTextures(1, &cubeTexture);
	glBindTexture(GL_TEXTURE_CUBE_MAP, cubeTexture);

	// Set up texture maps
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	// Load cube map images
	for (int i = 0; i < 6; ++i) {
		GLint iWidth = 0, iHeight = 0, iComponents = 0;
		GLenum eFormat;
		// Load this texture map
		GLbyte* pBytes = gltReadTGABits(szCubeFaces[i], &iWidth, &iHeight, &iComponents, &eFormat);
		glTexImage2D(cube_map[i], 0, iComponents, iWidth, iHeight, 0, eFormat, GL_UNSIGNED_BYTE, pBytes);
		free(pBytes);
	}
	glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

	viewFrame.MoveForward(-4.0f);
	gltMakeSphere(sphereBatch, 1.0f, 52, 26);
	gltMakeCube(cubeBatch, 20.0f);

	reflectionShader = gltLoadShaderPairWithAttributes("Reflection.vsh", "Reflection.fsh", 2, 
                                                  GLT_ATTRIBUTE_VERTEX, "vVertex", 
												  GLT_ATTRIBUTE_NORMAL, "vNormal");
	locMVPReflect = glGetUniformLocation(reflectionShader, "mvpMatrix");
	locMVReflect = glGetUniformLocation(reflectionShader, "mvMatrix");
	locNormalReflect = glGetUniformLocation(reflectionShader, "normalMatrix");
	locInvertedCamera = glGetUniformLocation(reflectionShader, "mInverseCamera");

	skyBoxShader = gltLoadShaderPairWithAttributes("SkyBox.vsh", "SkyBox.fsh", 2,
                                               GLT_ATTRIBUTE_VERTEX, "vVertex",
											   GLT_ATTRIBUTE_NORMAL, "vNormal");
	locMVPSkyBox = glGetUniformLocation(skyBoxShader, "mvpMatrix");
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
	if (key == GLUT_KEY_UP)
		viewFrame.MoveForward(0.1f);

	if (key == GLUT_KEY_DOWN)
		viewFrame.MoveForward(-0.1f);

	if (key == GLUT_KEY_LEFT)
		viewFrame.RotateLocalY(0.1);

	if (key == GLUT_KEY_RIGHT)
		viewFrame.RotateLocalY(-0.1);

	// Refresh the Window
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

	M3DMatrix44f mCamera;
	M3DMatrix44f mCameraRotOnly;
	M3DMatrix44f mInverseCamera;

	viewFrame.GetCameraMatrix(mCamera, false);
	viewFrame.GetMatrix(mCameraRotOnly, true);
	m3dInvertMatrix44(mInverseCamera, mCameraRotOnly);

	modelViewMatrix.PushMatrix();
	// Draw the sphere
	modelViewMatrix.MultMatrix(mCamera);
	glUseProgram(reflectionShader);
	glUniformMatrix4fv(locMVPReflect, 1, GL_FALSE, transformPipeline.GetMVPMatrix());
	glUniformMatrix4fv(locMVReflect, 1, GL_FALSE, transformPipeline.GetModelViewMatrix());
	glUniformMatrix3fv(locNormalReflect, 1, GL_FALSE, transformPipeline.GetNormalMatrix());
	glUniformMatrix4fv(locInvertedCamera, 1, GL_FALSE, mInverseCamera);

	glEnable(GL_CULL_FACE);
	sphereBatch.Draw();
	glDisable(GL_CULL_FACE);
	modelViewMatrix.PopMatrix();

	modelViewMatrix.PushMatrix();
	modelViewMatrix.MultMatrix(mCameraRotOnly);
	glUseProgram(skyBoxShader);
	glUniformMatrix4fv(locMVPSkyBox, 1, GL_FALSE, transformPipeline.GetMVPMatrix());
	cubeBatch.Draw();
	modelViewMatrix.PopMatrix();

	// Perform the buffer swap to display back buffer
	glutSwapBuffers();
	glutPostRedisplay();
}

/**
 * Cleanup... such as deleting texture objects
 */
void OnExit()
{
	glDeleteTextures(1, &cubeTexture);
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
