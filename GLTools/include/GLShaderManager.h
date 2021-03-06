#ifndef __GLT_SHADER_MANAGER
#define __GLT_SHADER_MANAGER


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

#include <vector>
using namespace std;

// Maximum length of shader name
#define MAX_SHADER_NAME_LENGTH	64


enum GLT_STOCK_SHADER { 
	GLT_SHADER_IDENTITY = 0,				// 单位着色器
	// 平面着色器：对定点不做任何变换，只传递这些定点并在默认的笛卡尔坐标系中对它们进行渲染
	GLT_SHADER_FLAT,						
	GLT_SHADER_SHADED,						// 上色着色器
	GLT_SHADER_DEFAULT_LIGHT,				// 默认光源着色器
	GLT_SHADER_POINT_LIGHT_DIFF,			// 点光源着色器
	GLT_SHADER_TEXTURE_REPLACE,				// 纹理替换矩阵
	GLT_SHADER_TEXTURE_MODULATE,			// 纹理调整着色器
	GLT_SHADER_TEXTURE_POINT_LIGHT_DIFF,	// 纹理光源着色器
	GLT_SHADER_TEXTURE_RECT_REPLACE,
	GLT_SHADER_LAST 
};

enum GLT_SHADER_ATTRIBUTE { 
	GLT_ATTRIBUTE_VERTEX = 0, 
	GLT_ATTRIBUTE_COLOR, 
	GLT_ATTRIBUTE_NORMAL, 
	GLT_ATTRIBUTE_TEXTURE0, 
	GLT_ATTRIBUTE_TEXTURE1, 
	GLT_ATTRIBUTE_TEXTURE2, 
	GLT_ATTRIBUTE_TEXTURE3, 
	GLT_ATTRIBUTE_LAST
};

typedef struct _ShaderLookupEntry {
	char		vertexShader[MAX_SHADER_NAME_LENGTH];
	char		fragShader[MAX_SHADER_NAME_LENGTH];
	GLuint	shaderId;
} ShaderLookupEntry;


class GLShaderManager
{
public:
	static GLShaderManager* Instance();

public:
	GLShaderManager(void);
	~GLShaderManager(void);

	// Call before using
	bool init(void);

	// Find one of the standard stock shaders and return it's shader handle. 
	GLuint GetStockShader(GLT_STOCK_SHADER nShaderID);

	// Use a stock shader, and pass in the parameters needed
	GLint useStockShader(GLT_STOCK_SHADER nShaderID, ...);

	// Load a shader pair from file, return NULL or shader handle. 
	// Vertex program name (minus file extension)
	// is saved in the lookup table
	GLuint loadWithFile(const char* szVertexFileName, const char *szFragFileName);

	// Load shaders from source text.
	GLuint LoadShaderPairSrc(const char *szName, const char *szVertexSrc, const char *szFragSrc);

	// Ditto above, but pop in the attributes
	GLuint LoadShaderPairWithAttributes(const char *szVertexProgFileName, const char *szFragmentProgFileName, ...);
	GLuint LoadShaderPairSrcWithAttributes(const char *szName, const char *szVertexProg, const char *szFragmentProg, ...);

	// Lookup a previously loaded shader
	GLuint lookupShader(const char *szVertexProg, const char *szFragProg = 0);

protected:
	GLuint	_shaderStock[GLT_SHADER_LAST];
	vector<ShaderLookupEntry>	_shaderEntryVec;

protected:
	static GLShaderManager* _this;
};


#endif
