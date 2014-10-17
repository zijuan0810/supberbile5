// openEXR
#include "IlmImf/ImfRgbaFile.h"
#include "IlmImf/ImfArray.h"

#include "GLTool-ext.h"


#ifdef _WIN32
	#pragma comment (lib, "Half.lib") 
	#pragma comment (lib, "Iex.lib")
	#pragma comment (lib, "IlmImf.lib")
	#pragma comment (lib, "IlmThread.lib")
	#pragma comment (lib, "Imath.lib")
	#pragma comment (lib, "zlib.lib")
#endif

#pragma warning (disable : 4305)

GLsizei	 screenWidth;			// Desired window or desktop width
GLsizei  screenHeight;			// Desired window or desktop height

GLboolean bFullScreen;			// Request to run full screen
GLboolean bAnimated;			// Request for continual updates

GLMatrixStack		modelViewMatrix;		// Modelview Matrix
GLMatrixStack		projectionMatrix;		// Projection Matrix
GLGeometryTransform	transformPipeline;		// Geometry Transform Pipeline
GLBatch             screenQuad;
GLBatch             fboQuad;
M3DMatrix44f        orthoMatrix;
M3DMatrix44f        fboOrthoMatrix;

GLuint				hdrTextures[1];
GLuint				lutTxtures[1];
GLuint				fboTextures[1];
GLuint				hdrTexturesWidth[1];
GLuint				hdrTexturesHeight[1];
GLuint				curHDRTex;
GLuint				fboName;
GLuint              mapTexProg;
GLuint              varExposureProg;
GLuint              adaptiveProg;
GLuint              curProg;
GLfloat				exposure;

void GenerateOrtho2DMat(GLuint windowWidth, GLuint windowHeight, GLuint imageWidth, GLuint imageHeight);
void SetupHDRProg();
void SetupStraightTexProg();
bool LoadOpenEXRImage(char *fileName, GLint textureName, GLuint &texWidth, GLuint &texHeight);

////////////////////////////////////////////////////////////////////////////
// Take a file name/location and load an OpenEXR
// Load the image into the "texture" texture object and pass back the texture sizes
// 
bool LoadOpenEXRImage(char *fileName, GLint textureName, GLuint &texWidth, GLuint &texHeight)
{
	// The OpenEXR uses exception handling to report errors or failures
	// Do all work in a try block to catch any thrown exceptions.
	try {
		//Imf::Array2D<Imf::Rgba> pixels;
		//Imf::RgbaInputFile file(fileName);
		//Imath::Box2i dw = file.dataWindow();
	}
	catch (Iex::BaseExc & e) {
		log_error("%s", e.what());
	}

	/*
	try {
		

		texWidth = dw.max.x - dw.min.x + 1;
		texHeight = dw.max.y - dw.min.y + 1;

		pixels.resizeErase(texHeight, texWidth);

		file.setFrameBuffer(&pixels[0][0] - dw.min.x - dw.min.y * texWidth, 1, texWidth);
		file.readPixels(dw.min.y, dw.max.y);

		GLfloat* texels = (GLfloat*)malloc(texWidth * texHeight * 3 * sizeof(GLfloat));
		GLfloat* pTex = texels;

		// Copy OpenEXR into local buffer for loading into a texture
		for (unsigned int v = 0; v < texHeight; v++) {
			for (unsigned int u = 0; u < texWidth; u++) {
				Imf::Rgba texel = pixels[texHeight - v - 1][u];
				pTex[0] = texel.r;
				pTex[1] = texel.g;
				pTex[2] = texel.b;

				pTex += 3;
			}
		}

		// Bind texture, load image, set tex state
		glBindTexture(GL_TEXTURE_2D, textureName);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, texWidth, texHeight, 0, GL_RGB, GL_FLOAT, texels);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

		free(texels);
	}
	catch (Iex::BaseExc & e) {
		log_error("%s", e.what());
	}
	*/

	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////
// Create a matrix that maps geometry to the screen. 1 unit in the x direction equals one pixel 
// of width, same with the y direction.
// It also depends on the size of the texture being displayed
void GenerateOrtho2DMat(GLuint windowWidth, GLuint windowHeight, GLuint imageWidth, GLuint imageHeight)
{
	float right = (float)windowWidth;
	float quadWidth = right;
	float left = 0.0f;
	float top = (float)windowHeight;
	float quadHeight = top;
	float bottom = 0.0f;
	float screenAspect = (float)windowWidth / windowHeight;
	float imageAspect = (float)imageWidth / imageHeight;

	if (screenAspect > imageAspect)
		quadWidth = windowHeight*imageAspect;
	else
		quadHeight = windowWidth*imageAspect;

	// set ortho matrix
	orthoMatrix[0] = (float)(2 / (right - left));
	orthoMatrix[1] = 0.0;
	orthoMatrix[2] = 0.0;
	orthoMatrix[3] = 0.0;

	orthoMatrix[4] = 0.0;
	orthoMatrix[5] = (float)(2 / (top - bottom));
	orthoMatrix[6] = 0.0;
	orthoMatrix[7] = 0.0;

	orthoMatrix[8] = 0.0;
	orthoMatrix[9] = 0.0;
	orthoMatrix[10] = (float)(-2 / (1.0 - 0.0));
	orthoMatrix[11] = 0.0;

	orthoMatrix[12] = -1 * (right + left) / (right - left);
	orthoMatrix[13] = -1 * (top + bottom) / (top - bottom);
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
	screenQuad.Vertex3f(quadWidth, 0.0f, 0.0f);

	screenQuad.Color4f(0.0f, 1.0f, 0.0f, 1.0f);
	screenQuad.MultiTexCoord2f(0, 0.0f, 1.0f);
	screenQuad.Vertex3f(0.0f, quadHeight, 0.0f);

	screenQuad.Color4f(0.0f, 1.0f, 0.0f, 1.0f);
	screenQuad.MultiTexCoord2f(0, 1.0f, 1.0f);
	screenQuad.Vertex3f(quadWidth, quadHeight, 0.0f);
	screenQuad.end();
}

void SetupStraightTexProg()
{
	// Set the cur prog for tex replace
	glUseProgram(mapTexProg);

	// Set MVP matrix
	glUniformMatrix4fv(glGetUniformLocation(mapTexProg, "mvpMatrix"), 1, GL_FALSE, transformPipeline.GetMVPMatrix());
}

void SetupHDRProg()
{
	// Set the program to the cur
	glUseProgram(curProg);

	// Set MVP matrix
	glUniformMatrix4fv(glGetUniformLocation(curProg, "mvpMatrix"), 1, GL_FALSE, transformPipeline.GetMVPMatrix());

	if (curProg == varExposureProg) {
		// Set the exposure for the scene
		glUniform1f(glGetUniformLocation(curProg, "exposure"), exposure);
	}
}

/**
 * Window has changed size, or has just been created. In either case, we need to use the window 
 * dimensions to set the viewport and the projection matrix.
 */
void ChangeSize(int w, int h)
{
	glViewport(0, 0, w, h);
	transformPipeline.setMatrixStacks(modelViewMatrix, projectionMatrix);

	modelViewMatrix.identity();

	screenWidth = w;
	screenHeight = h;

	GenerateOrtho2DMat(w, h, hdrTexturesWidth[curHDRTex], hdrTexturesHeight[curHDRTex]);
}

/**
 * This function does any needed initialization on the rendering context.
 * This is the first opportunity to do any OpenGL related tasks.
 */
void SetupRC()
{
	GLfloat texCoordOffsets[4][5 * 5 * 2];
	GLfloat exposureLUT[20] = { 11.0, 6.0, 3.2, 2.8, 2.2, 1.90, 1.80, 1.80, 1.70, 1.70, 1.60, 1.60, 1.50, 1.50, 1.40, 1.40, 1.30, 1.20, 1.10, 1.00 };

	// Will not use depth buffer
	glDisable(GL_DEPTH_TEST);
	curHDRTex = 0;

	// Init codel-view and leave it 
	modelViewMatrix.identity();

	// Black
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

	// Setup LUT texture for use with the adaptive exposure filter
	glGenTextures(1, lutTxtures);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_1D, lutTxtures[1]);
	glTexImage1D(GL_TEXTURE_1D, 0, GL_R16F, 20, 0, GL_RED, GL_FLOAT, exposureLUT);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	// Setup HDR texture(s)
	glActiveTexture(GL_TEXTURE0);
	glGenTextures(1, hdrTextures);
	glBindTexture(GL_TEXTURE_2D, hdrTextures[curHDRTex]);

	// Load HDR image from EXR file
	LoadOpenEXRImage("Tree.exr", hdrTextures[curHDRTex], hdrTexturesWidth[curHDRTex], hdrTexturesHeight[curHDRTex]);

	// Create ortho matrix and screen-sized quad matching images aspect ratio
	GenerateOrtho2DMat(screenWidth, screenHeight, hdrTexturesWidth[curHDRTex], hdrTexturesHeight[curHDRTex]);
	//GenerateFBOOrtho2DMat(hdrTexturesWidth[curHDRTex], hdrTexturesHeight[curHDRTex]);
	gltGenerateOrtho2DMat(hdrTexturesWidth[curHDRTex], hdrTexturesHeight[curHDRTex], fboOrthoMatrix, fboQuad);

	// Setup tex coords to be used for fetching HDR kernel data
	for (int k = 0; k < 4; k++) {
		float xInc = 1.0f / (GLfloat)(hdrTexturesWidth[curHDRTex] >> k);
		float yInc = 1.0f / (GLfloat)(hdrTexturesHeight[curHDRTex] >> k);

		for (int i = 0; i < 5; i++) {
			for (int j = 0; j < 5; j++) {
				texCoordOffsets[k][(((i * 5) + j) * 2) + 0] = (-1.0f * xInc) + ((GLfloat)i * xInc);
				texCoordOffsets[k][(((i * 5) + j) * 2) + 1] = (-1.0f * yInc) + ((GLfloat)j * yInc);
			}
		}
	}

	// Load shaders 
	mapTexProg = gltLoadShaderWithFileEx("hdr.vs", "hdr_simple.fs", 2, 
		GLT_ATTRIBUTE_VERTEX, "vVertex", GLT_ATTRIBUTE_TEXTURE0, "vTexCoord0");
	glBindFragDataLocation(mapTexProg, 0, "oColor");
	glLinkProgram(mapTexProg);

	varExposureProg = gltLoadShaderWithFileEx("hdr.vs", "hdr_exposure.fs", 2, 
		GLT_ATTRIBUTE_VERTEX, "vVertex", GLT_ATTRIBUTE_TEXTURE0, "vTexCoord0");
	glBindFragDataLocation(varExposureProg, 0, "oColor");
	glLinkProgram(varExposureProg);
	glUseProgram(varExposureProg);
	glUniform1i(glGetUniformLocation(varExposureProg, "textureUnit0"), 0);

	adaptiveProg = gltLoadShaderWithFileEx("hdr.vs", "hdr_adaptive.fs", 2, 
		GLT_ATTRIBUTE_VERTEX, "vVertex", GLT_ATTRIBUTE_TEXTURE0, "vTexCoord0");
	glBindFragDataLocation(adaptiveProg, 0, "oColor");
	glLinkProgram(adaptiveProg);
	glUseProgram(adaptiveProg);
	glUniform1i(glGetUniformLocation(adaptiveProg, "textureUnit0"), 0);
	glUniform1i(glGetUniformLocation(adaptiveProg, "textureUnit1"), 1);
	glUniform2fv(glGetUniformLocation(adaptiveProg, "tc_offset"), 25, texCoordOffsets[0]);

	glUseProgram(0);

	// Create and bind an FBO
	glGenFramebuffers(1, &fboName);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fboName);

	// Create the FBO texture
	glGenTextures(1, fboTextures);
	glBindTexture(GL_TEXTURE_2D, fboTextures[0]);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, hdrTexturesWidth[curHDRTex], hdrTexturesHeight[curHDRTex], 0, GL_RGBA, GL_FLOAT, NULL);
	glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, fboTextures[0], 0);

	// Make sure all went well
	gltCheckErrors();

	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

	// Set first running mode
	curProg = adaptiveProg;
}

/**
 * Respond to arrow keys by moving the camera frame of reference
 */
void SpecialKeys(int key, int x, int y)
{
	static CStopWatch timer;
	float fTime = timer.delta();
	float linear = fTime / 100;

	if (key == GLUT_KEY_UP) { // Increase the scene exposure
		if ((exposure + linear) < 20.0f) {
			exposure += linear;
		}
	}
	else if (key == GLUT_KEY_DOWN) { // Decrease the scene exposure
		if ((exposure - linear) > 0.01f)
			exposure -= linear;
	}
}

/**
 * Respond to the key pressed
 */
void KeyPressFunc(unsigned char key, int x, int y)
{
	if (key == '1') {
		curProg = mapTexProg;
	}
	else if (key == '2') {
		curProg = varExposureProg;
	}
	else if (key == '3') {
		curProg = adaptiveProg;
	}

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

	// first, draw to FBO at full FBO resolution

	// bind FBO with 8b attachment
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fboName);
	glViewport(0, 0, hdrTexturesWidth[curHDRTex], hdrTexturesHeight[curHDRTex]);
	glClear(GL_COLOR_BUFFER_BIT);

	// bind texture with HDR image
	glBindTexture(GL_TEXTURE_2D, hdrTextures[curHDRTex]);

	// render pass, downsample to 8b using selected program
	projectionMatrix.setMatrix(fboOrthoMatrix);
	SetupHDRProg();
	fboQuad.draw();

	// then draw the resulting image to the screen, maintain image proporions
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
	glViewport(0, 0, screenWidth, screenHeight);
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

	// attach 8b texture with HDR image
	glBindTexture(GL_TEXTURE_2D, fboTextures[0]);

	// draw screen sized, proportional quad with 8b texture
	projectionMatrix.setMatrix(orthoMatrix);
	SetupStraightTexProg();
	screenQuad.draw();

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

	glDeleteFramebuffers(1, &fboName);

	// Cleanup textures
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, 0);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_1D, 0);

	glDeleteTextures(1, hdrTextures);
	glDeleteTextures(1, lutTxtures);
	glDeleteTextures(1, fboTextures);
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
