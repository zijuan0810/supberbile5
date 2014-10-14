#ifndef __GL_BATCH__
#define __GL_BATCH__

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


#include "math3d.h"
#include "GLBatchBase.h"


class GLBatch : public GLBatchBase
{
public:
	GLBatch(void);
	virtual ~GLBatch(void);

	// Start populating(填充) the array
	void begin(GLenum primitive, GLuint nVerts, GLuint nTextureUnits = 0);

	// Tell the batch you are done
	void end(void);

	// Block Copy in vertex data
	void CopyVertexData3f(M3DVector3f *vVerts);
	void CopyNormalDataf(M3DVector3f *vNorms);
	void CopyColorData4f(M3DVector4f *vColors);
	void CopyTexCoordData2f(M3DVector2f *vTexCoords, GLuint uiTextureLayer);

	// Just to make life easier...
	inline void CopyVertexData3f(GLfloat *vVerts) { CopyVertexData3f((M3DVector3f *)(vVerts)); }
	inline void CopyNormalDataf(GLfloat *vNorms) { CopyNormalDataf((M3DVector3f *)(vNorms)); }
	inline void CopyColorData4f(GLfloat *vColors) { CopyColorData4f((M3DVector4f *)(vColors)); }
	inline void CopyTexCoordData2f(GLfloat *vTex, GLuint uiTextureLayer) { CopyTexCoordData2f((M3DVector2f *)(vTex), uiTextureLayer); }

	virtual void draw(void) override;

	// Immediate mode emulation
	// Slowest way to build an array on purpose... Use the above if you can instead
	void Reset(void);

	/**
	 * Add a single vertex to the end of the array
	 */
	void Vertex3f(GLfloat x, GLfloat y, GLfloat z);
	void Vertex3fv(M3DVector3f vVertex);

	/**
	 * Unlike normal OpenGL immediate mode, you must specify a normal per vertex
	 * or you will get junk...
	 */
	void Normal3f(GLfloat x, GLfloat y, GLfloat z);
	void Normal3fv(M3DVector3f vNormal);

	void Color4f(GLfloat r, GLfloat g, GLfloat b, GLfloat a);
	void Color4fv(M3DVector4f vColor);

	/**
	 * 绑定纹理
	 * @param texure 纹理对象id
	 * @param s s轴（水平）
	 * @param t t轴（垂直）
	 */
	void MultiTexCoord2f(GLuint texture, GLclampf s, GLclampf t);
	void MultiTexCoord2fv(GLuint texture, M3DVector2f vTexCoord);               

protected:
	GLenum	_primitiveType;		// What am I drawing....

	GLuint	_vertexBuf;		// 顶点数组
	GLuint	_normalBuf;
	GLuint	_colorBuf;
	GLuint*	_texCoordVec;
	GLuint	_vertexArrayObject;

	GLuint	_numVertsBuilding;	// Building up vertexes counter (immediate mode emulator)
	GLuint	_numVerts;			// Number of verticies in this batch
	GLuint	_numTextureUnits;		// Number of texture coordinate sets

	bool	_bBatchDone;			// Batch has been built

	M3DVector3f*	_vertex;
	M3DVector3f*	_normal;
	M3DVector4f*	_color;
	M3DVector2f**	_texCoords;
};

#endif // __GL_BATCH__
