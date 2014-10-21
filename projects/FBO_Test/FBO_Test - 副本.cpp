// Main.cpp

#pragma comment(lib, "sdl.lib")
#pragma comment(lib, "sdlmain.lib")
#pragma comment(lib, "opengl32.lib")
#pragma comment(lib, "glu32.lib")
#pragma comment(lib, "glew32.lib")

#include <iostream>
using namespace std;

#include "sdl.h"
#include "windows.h"
#include "GL/glew.h"
#include "GL/gl.h"
#include "GL/glu.h"

/****************************************************************************************************
* ȫ�ֱ�������
*****************************************************************************************************/
const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 600;
const int TEXTURE_WIDTH = 512;
const int TEXTURE_HEIGHT = 512;
const double NEAR_PLANE = 1.0f;
const double FAR_PLANE = 1000.0f;

SDL_Surface* drawContext;
SDL_Event event;

GLuint fbo = 0;		// FBO����ľ��
GLuint depthbuffer = 0;
GLuint rendertarget = 0;		// �������ľ��

/****************************************************************************************************
* ȫ�ֺ�������
*****************************************************************************************************/
void SetupWindow(void)
{
	// The window we'll be rendering to
	SDL_Window* window = nullptr;
	// The surface contained by the window
	SDL_Surface* screenSurface = nullptr;

	// initialize SDL
	if (SDL_Init(SDL_INIT_VIDEO) < 0) {
		printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
	}
	else {
		// create window
		window = SDL_CreateWindow("Render to Texture", SDL_WINDOWPOS_UNDEFINED,
			SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
		if (window == nullptr) {
			printf("Window could not be created! SDL_Error: %s\n", SDL_GetError());
		}
		else {
			// Get window surface 
			screenSurface = SDL_GetWindowSurface(window);

			// fill the surface white
			SDL_FillRect(screenSurface, nullptr, 
				SDL_MapRGB(screenSurface->format, 0xFF, 0xFF, 0xFF));
			// update the surface
			SDL_UpdateWindowSurface(window);
			// wait two seconds
			SDL_Delay(2000);
		}
	}

	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 16);
	SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);

	drawContext = screenSurface;

	GLenum err = glewInit(); // GLEW�ĳ�ʼ��������OpenGL�����ı�����֮�����
	if (err != GLEW_OK)
	{
		SDL_Quit();
	}
}

// ��ʼ�������
void SetupCamera(void)
{
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(45, (double)SCREEN_WIDTH / (double)SCREEN_HEIGHT, NEAR_PLANE, FAR_PLANE);
	gluLookAt(5, 5, 5, 0, 0, 0, 0, 1, 0);

	// ���ֱ任Ӧ����GL_MODELVIEWģʽ�½���
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	// Z-buffer
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);

	// ����2D��ͼ
	glEnable(GL_TEXTURE_2D);
}

// ��ʼ����������
void SetupResource(void)
{
	// ��������
	glGenTextures(1, &rendertarget);
	glBindTexture(GL_TEXTURE_2D, rendertarget);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, TEXTURE_WIDTH, TEXTURE_HEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
	glBindTexture(GL_TEXTURE_2D, 0);

	// ������Ȼ�����
	glGenRenderbuffersEXT(1, &depthbuffer);
	glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, depthbuffer);
	glRenderbufferStorageEXT(GL_RENDERBUFFER_EXT, GL_DEPTH_COMPONENT, TEXTURE_WIDTH, TEXTURE_HEIGHT);
	glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, 0);

	// ����FBO����
	glGenFramebuffersEXT(1, &fbo);
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fbo);
	glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, rendertarget, 0);
	glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_RENDERBUFFER_EXT, depthbuffer);
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);

	GLenum status = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
	if (status != GL_FRAMEBUFFER_COMPLETE_EXT)
	{
		SDL_Quit();
	}
}

// ��Ⱦ������
void Render(void)
{
	// ��Ĭ��FBO������֡��������ID��0��
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
	glBindTexture(GL_TEXTURE_2D, rendertarget);
	glViewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);

	// ��Ⱦ
	glClearColor(0, 0, 1, 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glBegin(GL_POLYGON);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glColor3f(1, 1, 1);

	glTexCoord2f(1, 1);
	glVertex3d(1, 1, 0);

	glTexCoord2f(0, 1);
	glVertex3d(-1, 1, 0);

	glTexCoord2f(0, 0);
	glVertex3d(-1, -1, 0);

	glTexCoord2f(1, 0);
	glVertex3d(1, -1, 0);

	glEnd();

	// ��תǰ�󻺳���
	SDL_GL_SwapBuffers();
}

// ��Ⱦ������
void RenderToTarget(void)
{
	glBindTexture(GL_TEXTURE_2D, 0); // ȡ���󶨣���Ϊ�����ȡ������Ⱦ�������ʱ���ʹ��������

	// ����ȾĿ��
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fbo);
	glViewport(0, 0, TEXTURE_WIDTH, TEXTURE_HEIGHT);

	// ��Ⱦ
	glClearColor(1, 1, 0, 1);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glBegin(GL_POLYGON);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glColor4f(1, 0, 0, 1);
	glVertex3d(0, 1, 0);
	glVertex3d(-1, -1, 0);
	glVertex3d(1, -1, 0);

	glEnd();

	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
}

void Clear(void)
{

}
/****************************************************************************************************
* ���������
*****************************************************************************************************/
int main(int argc, char* argv[])
{
	SetupWindow();
	SetupCamera();
	SetupResource();

	// ���ð�����������
	SDL_EnableKeyRepeat(16, SDL_DEFAULT_REPEAT_INTERVAL);

	for (;;)
	{
		if (SDL_PollEvent(&event) == 0)
		{
			RenderToTarget();
			Render();
		}
		else
		{
			if (event.type == SDL_QUIT)
			{
				break;
			}
		}
	}

	SDL_Quit();
	return 0;
}