#include <GLTools.h>
#include <GL/glut.h>
#include <GLShaderManager.h>

const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 600;
const int TEXTURE_WIDTH = 512;
const int TEXTURE_HEIGHT = 512;
const double NEAR_PLANE = 1.0f;
const double FAR_PLANE = 1000.0f;

GLuint fbo = 0;		// FBO对象的句柄
GLuint depthbuffer = 0;
GLuint textureId = 0;		// 纹理对象的句柄

// 初始化几何形体
void SetupResource(void)
{
	// 创建纹理
	glGenTextures(1, &textureId);
	glBindTexture(GL_TEXTURE_2D, textureId);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, TEXTURE_WIDTH, TEXTURE_HEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
	glBindTexture(GL_TEXTURE_2D, 0);

	// 创建深度缓冲区
	glGenRenderbuffers(1, &depthbuffer);
	glBindRenderbuffer(GL_RENDERBUFFER, depthbuffer); // target must be GL_RENDERBUFFER
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, TEXTURE_WIDTH, TEXTURE_HEIGHT);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);

	// 创建FBO对象
	glGenFramebuffers(1, &fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureId, GLT_MIPMAP_LEVEL_0); // GL_FRAMEBUFFER is equivalent to GL_DRAW_FRAMEBUFFER
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthbuffer); // 绑定RBO
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (status != GL_FRAMEBUFFER_COMPLETE) {
		exit(0);
	}
}

// 渲染到纹理
void RenderToTexture(void)
{
	glBindTexture(GL_TEXTURE_2D, 0); // 取消绑定，因为如果不取消，渲染到纹理的时候会使用纹理本身

	// 绑定渲染目标
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
	glViewport(0, 0, TEXTURE_WIDTH, TEXTURE_HEIGHT);

	// clear color Green
	glClearColor(0, 1, 0, 1);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glBegin(GL_POLYGON);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	// create the red triangle
	glColor4f(1, 0, 0, 1);
	glVertex3d(0, 1, 0);
	glVertex3d(-1, -1, 0);
	glVertex3d(1, -1, 0);

	glEnd();

	// 删除当前绑定的FBO，由于窗体系统创建的FBO的ID默认为0，这里相当于将FBO恢复到默认
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}


// Window has changed size, or has just been created. In either case, we need
// to use the window dimensions to set the viewport and the projection matrix.
void ChangeSize(int w, int h)
{
	//glViewport(0, 0, w, h);
}

// This function does any needed initialization on the rendering context. 
// This is the first opportunity to do any OpenGL related tasks.
void SetupRC()
{
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(45, (double)SCREEN_WIDTH / (double)SCREEN_HEIGHT, NEAR_PLANE, FAR_PLANE);
	gluLookAt(5, 5, 5, 0, 0, 0, 0, 1, 0);

	// 各种变换应该在GL_MODELVIEW模式下进行
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	// Z-buffer
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);

	// 启用2D贴图
	glEnable(GL_TEXTURE_2D);

	SetupResource();
}

// Respond to arrow keys by moving the camera frame of reference
void SpecialKeys(int key, int x, int y)
{
}

// Called to draw scene
void RenderScene(void)
{
	// Clear the window with current clearing color
	//glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	RenderToTexture();

	// 绑定默认FBO（窗体帧缓冲区的ID是0）
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glBindTexture(GL_TEXTURE_2D, textureId);
	glViewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);

	// 渲染
	glClearColor(0, 0, 0, 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glBegin(GL_POLYGON);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glColor3f(1, 1, 1);

	// 开始纹理贴图
	glTexCoord2f(1, 1);
	glVertex3d(1, 1, 0);

	glTexCoord2f(0, 1);
	glVertex3d(-1, 1, 0);

	glTexCoord2f(0, 0);
	glVertex3d(-1, -1, 0);

	glTexCoord2f(1, 0);
	glVertex3d(1, -1, 0);

	glEnd();

	// Perform the buffer swap to display back buffer
	glutSwapBuffers();
}

// Main entry point for GLUT based programs
int main(int argc, char* argv[])
{
	gltSetWorkingDirectory(argv[0]);

	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH | GLUT_STENCIL);
	glutInitWindowSize(800, 600);
	glutCreateWindow("Triangle");

	glutReshapeFunc(ChangeSize);
	glutDisplayFunc(RenderScene);
	glutSpecialFunc(SpecialKeys);

	GLenum err = glewInit();
	if (GLEW_OK != err) {
		fprintf(stderr, "GLEW Error: %s\n", glewGetErrorString(err));
		return 1;
	}

	SetupRC();

	glutMainLoop();
	return 0;
}
