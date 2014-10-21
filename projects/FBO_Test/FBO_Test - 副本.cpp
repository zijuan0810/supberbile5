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
* 全局变量定义
*****************************************************************************************************/
const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 600;
const int TEXTURE_WIDTH = 512;
const int TEXTURE_HEIGHT = 512;
const double NEAR_PLANE = 1.0f;
const double FAR_PLANE = 1000.0f;

SDL_Surface* drawContext;
SDL_Event event;

GLuint fbo = 0;		// FBO对象的句柄
GLuint depthbuffer = 0;
GLuint rendertarget = 0;		// 纹理对象的句柄

/****************************************************************************************************
* 全局函数定义
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

	GLenum err = glewInit(); // GLEW的初始化必须在OpenGL上下文被创建之后调用
	if (err != GLEW_OK)
	{
		SDL_Quit();
	}
}

// 初始化摄像机
void SetupCamera(void)
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
}

// 初始化几何形体
void SetupResource(void)
{
	// 创建纹理
	glGenTextures(1, &rendertarget);
	glBindTexture(GL_TEXTURE_2D, rendertarget);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, TEXTURE_WIDTH, TEXTURE_HEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
	glBindTexture(GL_TEXTURE_2D, 0);

	// 创建深度缓冲区
	glGenRenderbuffersEXT(1, &depthbuffer);
	glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, depthbuffer);
	glRenderbufferStorageEXT(GL_RENDERBUFFER_EXT, GL_DEPTH_COMPONENT, TEXTURE_WIDTH, TEXTURE_HEIGHT);
	glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, 0);

	// 创建FBO对象
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

// 渲染到窗体
void Render(void)
{
	// 绑定默认FBO（窗体帧缓冲区的ID是0）
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
	glBindTexture(GL_TEXTURE_2D, rendertarget);
	glViewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);

	// 渲染
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

	// 翻转前后缓冲区
	SDL_GL_SwapBuffers();
}

// 渲染到纹理
void RenderToTarget(void)
{
	glBindTexture(GL_TEXTURE_2D, 0); // 取消绑定，因为如果不取消，渲染到纹理的时候会使用纹理本身

	// 绑定渲染目标
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fbo);
	glViewport(0, 0, TEXTURE_WIDTH, TEXTURE_HEIGHT);

	// 渲染
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
* 主程序入口
*****************************************************************************************************/
int main(int argc, char* argv[])
{
	SetupWindow();
	SetupCamera();
	SetupResource();

	// 启用按键联动功能
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