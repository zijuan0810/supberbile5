#ifndef __GLTOOLS__LIBRARY
#define __GLTOOLS__LIBRARY


// There is a static block allocated for loading shaders to 
// prevent heap fragmentation
#define MAX_SHADER_LENGTH   8192


// Bring in OpenGL 
// Windows
#ifdef WIN32
#include <windows.h>		// Must have for Windows platform builds
#ifndef GLEW_STATIC
#define GLEW_STATIC
#endif

#include <gl\glew.h>			// OpenGL Extension "autoloader"
#include <gl\gl.h>			// Microsoft OpenGL headers (version 1.1 by themselves)
#endif

// Mac OS X
#ifdef __APPLE__
#include <stdlib.h>

#include <TargetConditionals.h>
#if TARGET_OS_IPHONE | TARGET_IPHONE_SIMULATOR
#include <OpenGLES/ES2/gl.h>
#include <OpenGLES/ES2/glext.h>
#define OPENGL_ES
#else
#include <GL/glew.h>
#include <OpenGL/gl.h>		// Apple OpenGL haders (version depends on OS X SDK version)
#endif
#endif

// Linux
#ifdef linux
#define GLEW_STATIC
#include <glew.h>
#endif

//////////////////////// TEMPORARY TEMPORARY TEMPORARY - On SnowLeopard this is suppored, but GLEW doens't hook up properly
//////////////////////// Fixed probably in 10.6.3
#ifdef __APPLE__
#define glGenVertexArrays glGenVertexArraysAPPLE
#define glDeleteVertexArrays  glDeleteVertexArraysAPPLE
#define glBindVertexArray	glBindVertexArrayAPPLE
#ifndef OPENGL_ES
#define glGenerateMipmap    glGenerateMipmapEXT
#endif
#endif


// Universal includes
#include <stdio.h>
#include <math.h>
#include "math3d.h"
#include "GLBatch.h"
#include "GLTriangleBatch.h"

   
///////////////////////////////////////////////////////
// Macros for big/little endian happiness
// These are intentionally written to be easy to understand what they 
// are doing... no flames please on the inefficiency of these.
#ifdef __BIG_ENDIAN__
///////////////////////////////////////////////////////////
// This function says, "this pointer is a little endian value"
// If the value must be changed it is... otherwise, this
// function is defined away below (on Intel systems for example)
inline void LITTLE_ENDIAN_WORD(void *pWord)
	{
    unsigned char *pBytes = (unsigned char *)pWord;
    unsigned char temp;
    
    temp = pBytes[0];
    pBytes[0] = pBytes[1];
    pBytes[1] = temp;
	}

///////////////////////////////////////////////////////////
// This function says, "this pointer is a little endian value"
// If the value must be changed it is... otherwise, this
// function is defined away below (on Intel systems for example)
inline void LITTLE_ENDIAN_DWORD(void *pWord)
	{
    unsigned char *pBytes = (unsigned char *)pWord;
    unsigned char temp;
    
    // Swap outer bytes
    temp = pBytes[3];
    pBytes[3] = pBytes[0];
    pBytes[0] = temp;
    
    // Swap inner bytes
    temp = pBytes[1];
    pBytes[1] = pBytes[2];
    pBytes[2] = temp;
	}
#else

// Define them away on little endian systems
#define LITTLE_ENDIAN_WORD 
#define LITTLE_ENDIAN_DWORD 
#endif


///////////////////////////////////////////////////////////////////////////////
//         THE LIBRARY....
///////////////////////////////////////////////////////////////////////////////


// Print OpenGL version information
extern void gltPrintOpenGLInfo();

// Get the OpenGL version
void gltGetOpenGLVersion(GLint &nMajor, GLint &nMinor);

/**
 * Check to see if an exension is supported
 */
bool gltIsExtSupported(const char *szExtension);

// Set working directoyr to /Resources on the Mac
void gltSetWorkingDirectory(const char *szArgv);

///////////////////////////////////////////////////////////////////////////////
GLbyte* gltReadBMPBits(const char *szFileName, int *nWidth, int *nHeight);

/**
 * Load a .TGA file from the hard disk
 */
GLbyte *gltReadTGABits(const char *szFileName, GLint *iWidth, GLint *iHeight, GLint *iComponents, 
					   GLenum *eFormat);

// Capture the frame buffer and write it as a .tga
// Does not work on the iPhone
#ifndef OPENGL_ES
GLint gltGrabScreenTGA(const char *szFileName);
#endif


/**
 * Create a torus(圆环)
 * @param torusBatch 创建的batch
 * @param majorRaius the radius from the center to the outer edge of the torus
 * @param minorRadius the radius to the inner edge
 * @param iSlices 指围绕着球体排列的三角形数量
 * @param iStacks 指从球体底部堆叠到顶部的三角形带数量
 */
void gltMakeTorus(GLTriangleBatch& torusBatch, GLfloat majorRadius, GLfloat minorRadius, 
				  GLint iSlices, GLint iStacks);

/**
 * Create a sphere
 * @param sphereBatch 创建的batch
 * @param fRadius 球半径
 * @param iSlices 指围绕着球体排列的三角形数量
 * @param iStacks 指从球体底部堆叠到顶部的三角形带数量
 */
void gltMakeSphere(GLTriangleBatch& sphereBatch, GLfloat fRadius, GLint iSlices, GLint iStacks);

/**
 * Create a disk(圆盘)
 * @param diskBatch 创建的batch
 */
void gltMakeDisk(GLTriangleBatch& diskBatch, GLfloat innerRadius, GLfloat outerRadius, 
				 GLint iSlices, GLint iStacks);

/**
 * Create a cylinder(圆筒)
 * @param cylinderBatch 创建的batch
 * @param baseRadius The base radius
 * @param topRadius The top radius
 * @param fLength The hight of the cylinder
 * @param iSlices The number of triangle pairs that circle the z-axis
 * @param iStacks The number of rings stacked from the bottom to the top of the cylinder
 */
void gltMakeCylinder(GLTriangleBatch& cylinderBatch, GLfloat baseRadius, GLfloat topRadius, 
					 GLfloat fLength, GLint iSlices, GLint iStacks);

/**
 * Make a cube, centered at the origin, and with a specified "radius"
 */
void gltMakeCube(GLBatch& cubeBatch, GLfloat fRadius);

/** 
* Load the shader from the source text
*/
void gltLoadShaderSrc(const char *szShaderSrc, GLuint shader);

/** 
* Load the shader from the specified file. Returns false if the
* shader could not be loaded
*/
bool gltLoadShaderFile(const char *szFile, GLuint shader);

GLuint gltLoadShaderPair(const char *szVertexProg, const char *szFragmentProg);
/** 
* Load a pair of shaders, compile, and link together. Specify the complete
* source text for each shader. After the shader names, specify the number
* of attributes, followed by the index and attribute name of each attribute
*/
GLuint gltLoadShaderPairWithAttributes(const char *szVertexProg, const char *szFragmentProg, ...);

GLuint gltLoadShaderPairSrc(const char *szVertexSrc, const char *szFragmentSrc);
GLuint gltLoadShaderPairSrcWithAttributes(const char *szVertexProg, const char *szFragmentProg, ...);

bool gltCheckErrors(GLuint progName = 0);
void gltGenerateOrtho2DMat(GLuint width, GLuint height, M3DMatrix44f &orthoMatrix, GLBatch &screenQuad);

/**
 * Show OpenGL some information
 */
void gltShowVersionInfo();


/**
 * Load a TGA as a 2D texture. Completely initialize the state
 */
bool gltLoadTextureTGA(const char* pszFileName, GLenum minFilter, GLenum magFilter, GLenum warpMode);

bool gltLoadTextureTGARect(const char* pszFileName, GLenum minFilter, GLenum magFilter, GLenum warpMode);

#endif
