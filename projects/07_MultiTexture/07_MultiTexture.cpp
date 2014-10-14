#include "GLTool-ext.h"

GLFrame	 viewFrame;
GLFrustum viewFrustum;
GLGeometryTransform transformPipeline;

GLBatch			skyboxBatch;
GLTriangleBatch	sphereBatch;

GLMatrixStack       modelViewMatrix;
GLMatrixStack       projectionMatrix;

GLuint	cubeTexture;
GLuint	tarnishTexture;
GLint	reflectionShader;
GLint	skyBoxShader;

GLint locMVPReflect, locMVReflect, locNormalReflect, locInvertedCamera;
GLint locCubeMap, locTarnishMap;
GLint locMVPSkyBox;


// Six sides of a cube map
const char* szCubeFaces[6] = { 
	"pos_x.tga", "neg_x.tga", 
	"pos_y.tga", "neg_y.tga", 
	"pos_z.tga", "neg_z.tga",
};

GLenum Texture_Cube_Maps[6] = { 
	GL_TEXTURE_CUBE_MAP_POSITIVE_X,
	GL_TEXTURE_CUBE_MAP_NEGATIVE_X,
	GL_TEXTURE_CUBE_MAP_POSITIVE_Y,
	GL_TEXTURE_CUBE_MAP_NEGATIVE_Y,
	GL_TEXTURE_CUBE_MAP_POSITIVE_Z,
	GL_TEXTURE_CUBE_MAP_NEGATIVE_Z,
};


/**
 * Window has changed size, or has just been created. In either case, we need to use the window
 * dimensions to set the viewport and the projection matrix.
 */
void ChangeSize(int w, int h)
{
	h = (h == 0) ? 1 : h;

	// Set Viewport to window dimensions
	glViewport(0, 0, w, h);

	viewFrustum.SetPerspective(35.0f, float(w) / float(h), 1.0f, 1000.0f);

	projectionMatrix.setMatrix(viewFrustum.GetProjectionMatrix());
	transformPipeline.SetMatrixStacks(modelViewMatrix, projectionMatrix);
}

/**
 * This function does any needed initialization on the rendering context.
 * This is the first opportunity to do any OpenGL related tasks.
 */
void SetupRC()
{
	// Blue background
	glClearColor(0.0f, 0.0f, 1.0f, 1.0f );

	// Cull backs of polygons
	glCullFace(GL_BACK);
	glFrontFace(GL_CCW);
	glEnable(GL_DEPTH_TEST);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	// Load the tannish texture
	glGenTextures(1, &tarnishTexture);
	glBindTexture(GL_TEXTURE_2D, tarnishTexture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	GLint iWidth, iHeight, iComponents;
	GLenum eFormat;
	GLbyte* texBytes = gltReadTGABits("tarnish.tga", &iWidth, &iHeight, &iComponents, &eFormat);
	glTexImage2D(GL_TEXTURE_2D, 0, iComponents, iWidth, iHeight, 0, eFormat, GL_UNSIGNED_BYTE, texBytes);
	free(texBytes);
	glGenerateMipmap(GL_TEXTURE_2D);

	// Load the cube map
	glGenTextures(1, &cubeTexture);
	glBindTexture(GL_TEXTURE_CUBE_MAP, cubeTexture);

	// Set up texture maps
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	// Load Cube Map images
	for (int i = 0; i < 6; ++i) {
		// Load this texture map
		GLint iWidth, iHeight, iComponents;
		GLenum eFormat;
		GLbyte* bytes = gltReadTGABits(szCubeFaces[i], &iWidth, &iHeight, &iComponents, &eFormat);
		glTexImage2D(Texture_Cube_Maps[i], 0, iComponents, iWidth, iHeight, 0, eFormat, GL_UNSIGNED_BYTE, bytes);
		free(bytes);
	}
	glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

	viewFrame.MoveForward(-4.0f);
	gltCreateSphere(sphereBatch, 1.0f, 52, 26);
	gltMakeCube(skyboxBatch, 20.0f);

	reflectionShader = gltLoadShaderWithFileEx(
		"Reflection.vs.glsl", "Reflection.fs.glsl", 3,
		GLT_ATTRIBUTE_VERTEX, "vVertex",
		GLT_ATTRIBUTE_NORMAL, "vNormal",
		GLT_ATTRIBUTE_TEXTURE0, "vTexCoords");

	locMVPReflect = glGetUniformLocation(reflectionShader, "mvpMatrix");
	locMVReflect = glGetUniformLocation(reflectionShader, "mvMatrix");
	locNormalReflect = glGetUniformLocation(reflectionShader, "normalMatrix");
	locInvertedCamera = glGetUniformLocation(reflectionShader, "mInverseCamera");
	locCubeMap = glGetUniformLocation(reflectionShader, "cubeMap");
	locTarnishMap = glGetUniformLocation(reflectionShader, "tarnishMap");

	skyBoxShader = gltLoadShaderWithFileEx(
		"SkyBox.vs.glsl", "SkyBox.fs.glsl", 2,
		GLT_ATTRIBUTE_VERTEX, "vVertex",
		GLT_ATTRIBUTE_NORMAL, "vNormal");

	locMVPSkyBox = glGetUniformLocation(skyBoxShader, "mvpMatrix");

	// Set textures to their texture units
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, tarnishTexture);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, cubeTexture);
}

/**
 * Respond to arrow keys by moving the camera frame of reference
 */
void SpecialKeys(int key, int x, int y)
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
 * Respond to the key pressed
 */
void KeyPressFunc(unsigned char key, int x, int y)
{
	switch (key) {
	case KEY_A:
		glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
		break;
	case KEY_B:
		glDisable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
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
	// Clear the window with current clearing color
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	M3DMatrix44f mCamera;
	M3DMatrix44f mCameraRotOnly;
	M3DMatrix44f mInverseCamera;

	viewFrame.GetCameraMatrix(mCamera, false);
	viewFrame.GetCameraMatrix(mCameraRotOnly, true);
	m3dInvertMatrix44(mInverseCamera, mCameraRotOnly);

	modelViewMatrix.push();
		// draw the sphere
		modelViewMatrix.MultMatrix(mCamera);
		glUseProgram(reflectionShader);
		glUniformMatrix4fv(locMVPReflect, 1, GL_FALSE, transformPipeline.GetMVPMatrix());
		glUniformMatrix4fv(locMVReflect, 1, GL_FALSE, transformPipeline.GetModelViewMatrix());
		glUniformMatrix3fv(locNormalReflect, 1, GL_FALSE, transformPipeline.GetNormalMatrix());
		glUniformMatrix4fv(locInvertedCamera, 1, GL_FALSE, mInverseCamera);
		glUniform1i(locCubeMap, 0);
		glUniform1i(locTarnishMap, 1);

		glEnable(GL_CULL_FACE);
		sphereBatch.draw();
		glDisable(GL_CULL_FACE);
	modelViewMatrix.pop();

	modelViewMatrix.push();
		modelViewMatrix.MultMatrix(mCameraRotOnly);
		glUseProgram(skyBoxShader);
		glUniformMatrix4fv(locMVPSkyBox, 1, GL_FALSE, transformPipeline.GetMVPMatrix());
		skyboxBatch.draw();
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
	glDeleteTextures(1, &cubeTexture);
	glDeleteTextures(1, &tarnishTexture);
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
