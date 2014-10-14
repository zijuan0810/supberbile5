#include "GLShaderManager.h"
#include "GLTools.h"
#include "stdarg.h"

// Stock Shader Source Code

/*
* Identity Shader (GLT_SHADER_IDENTITY)
* This shader does no transformations at all, and uses the current
* glColor value for fragments.
* It will shade between verticies.
*/
static const char *szIdentityShaderVP = "	\
	attribute vec4 vVertex;					\
	void main(void) {						\
		gl_Position = vVertex;				\
	}";

static const char *szIdentityShaderFP = ""
#ifdef OPENGL_ES
	"precision mediump float;"
#endif
	"uniform vec4 vColor;		"
	"void main(void) {			" 
	"	gl_FragColor = vColor;	"
	"}";

/* 
* Flat Shader (GLT_SHADER_FLAT)
* This shader applies the given model view matrix to the verticies, 
* and uses a uniform color value.
*/
static const char *szFlatShaderVP = "		\
	uniform mat4 mvpMatrix;					\
	attribute vec4 vVertex;					\
	void main(void)	{						\
		gl_Position = mvpMatrix * vVertex;	\
	}";

static const char *szFlatShaderFP = 
#ifdef OPENGL_ES
	"precision mediump float;"
#endif
	"uniform vec4 vColor;"
	"void main(void) "
	"{ gl_FragColor = vColor; "
	"}";

/** 
* GLT_SHADER_SHADED
* Point light, diffuse lighting only
*/
static const char *szShadedVP =		
	"uniform mat4 mvpMatrix;"
	"attribute vec4 vColor;"
	"attribute vec4 vVertex;"
	"varying vec4 vFragColor;"
	"void main(void) {"
	"vFragColor = vColor; "
	" gl_Position = mvpMatrix * vVertex; "
	"}";

static const char *szShadedFP =     
#ifdef OPENGL_ES
	"precision mediump float;"
#endif
	"varying vec4 vFragColor; "
	"void main(void) { "
	" gl_FragColor = vFragColor; "
	"}";

/** 
* GLT_SHADER_DEFAULT_LIGHT
* Simple diffuse, directional, and vertex based light
*/
static const char *szDefaultLightVP = 
	"uniform mat4 mvMatrix;"
	"uniform mat4 pMatrix;"
	"varying vec4 vFragColor;"
	"attribute vec4 vVertex;"
	"attribute vec3 vNormal;"
	"uniform vec4 vColor;"
	"void main(void) { "
	" mat3 mNormalMatrix;"
	" mNormalMatrix[0] = mvMatrix[0].xyz;"
	" mNormalMatrix[1] = mvMatrix[1].xyz;"
	" mNormalMatrix[2] = mvMatrix[2].xyz;"
	" vec3 vNorm = normalize(mNormalMatrix * vNormal);"
	" vec3 vLightDir = vec3(0.0, 0.0, 1.0); "
	" float fDot = max(0.0, dot(vNorm, vLightDir)); "
	" vFragColor.rgb = vColor.rgb * fDot;"
	" vFragColor.a = vColor.a;"
	" mat4 mvpMatrix;"
	" mvpMatrix = pMatrix * mvMatrix;"
	" gl_Position = mvpMatrix * vVertex; "
	"}";


static const char *szDefaultLightFP =	
#ifdef OPENGL_ES
	"precision mediump float;"
#endif
	"varying vec4 vFragColor; "
	"void main(void) { "
	" gl_FragColor = vFragColor; "
	"}";

/** 
* GLT_SHADER_POINT_LIGHT_DIFF
* Point light, diffuse lighting only
*/
static const char *szPointLightDiffVP =	  " \
	uniform mat4 mvMatrix;	 \
	uniform mat4 pMatrix;	\
	uniform vec3 vLightPos;	\
	uniform vec4 vColor;\
	attribute vec4 vVertex;\
	attribute vec3 vNormal;\
	varying vec4 vFragColor;\
	void main(void) { \
	 mat3 mNormalMatrix;\
	 mNormalMatrix[0] = normalize(mvMatrix[0].xyz);\
	 mNormalMatrix[1] = normalize(mvMatrix[1].xyz);\
	 mNormalMatrix[2] = normalize(mvMatrix[2].xyz);\
	 vec3 vNorm = normalize(mNormalMatrix * vNormal);\
	 vec4 ecPosition;\
	 vec3 ecPosition3;\
	 ecPosition = mvMatrix * vVertex;\
	 ecPosition3 = ecPosition.xyz /ecPosition.w;\
	 vec3 vLightDir = normalize(vLightPos - ecPosition3);\
	 float fDot = max(0.0, dot(vNorm, vLightDir)); \
	 vFragColor.rgb = vColor.rgb * fDot;\
	 vFragColor.a = vColor.a;\
	 mat4 mvpMatrix;\
	 mvpMatrix = pMatrix * mvMatrix;\
	 gl_Position = mvpMatrix * vVertex; \
	}";


static const char *szPointLightDiffFP = 
#ifdef OPENGL_ES
	"precision mediump float;"
#endif
	"varying vec4 vFragColor; "
	"void main(void) { "
	" gl_FragColor = vFragColor; "
	"}";

/** 
* GLT_SHADER_TEXTURE_REPLACE
* Just put the texture on the polygons
*/
static const char *szTextureReplaceVP =	
	"uniform mat4 mvpMatrix;"
	"attribute vec4 vVertex;"
	"attribute vec2 vTexCoord0;"
	"varying vec2 vTex;"
	"void main(void) "
	"{ vTex = vTexCoord0;" 
	" gl_Position = mvpMatrix * vVertex; "
	"}";

static const char *szTextureReplaceFP = 
#ifdef OPENGL_ES
	"precision mediump float;"
#endif
	"varying vec2 vTex;"
	"uniform sampler2D textureUnit0;"
	"void main(void) "
	"{ gl_FragColor = texture2D(textureUnit0, vTex); "
	"}";

/** 
* Just put the texture on the polygons
*/
static const char *szTextureRectReplaceVP =	"uniform mat4 mvpMatrix;"
	"attribute vec4 vVertex;"
	"attribute vec2 vTexCoord0;"
	"varying vec2 vTex;"
	"void main(void) "
	"{ vTex = vTexCoord0;" 
	" gl_Position = mvpMatrix * vVertex; "
	"}";

static const char *szTextureRectReplaceFP = 
#ifdef OPENGL_ES
	"precision mediump float;"
#endif
	"varying vec2 vTex;"
	"uniform sampler2DRect textureUnit0;"
	"void main(void) "
	"{ gl_FragColor = texture2DRect(textureUnit0, vTex); "
	"}";

/** 
* GLT_SHADER_TEXTURE_MODULATE
* Just put the texture on the polygons, but multiply by the color (as a unifomr)
*/
static const char *szTextureModulateVP ="uniform mat4 mvpMatrix;"
	"attribute vec4 vVertex;"
	"attribute vec2 vTexCoord0;"
	"varying vec2 vTex;"
	"void main(void) "
	"{ vTex = vTexCoord0;" 
	" gl_Position = mvpMatrix * vVertex; "
	"}";

static const char *szTextureModulateFP =
#ifdef OPENGL_ES
	"precision mediump float;"
#endif
	"varying vec2 vTex;"
	"uniform sampler2D textureUnit0;"
	"uniform vec4 vColor;"
	"void main(void) "
	"{ gl_FragColor = vColor * texture2D(textureUnit0, vTex); "
	"}";

/** 
* GLT_SHADER_TEXTURE_POINT_LIGHT_DIFF
* Point light (Diffuse only), with texture (modulated)
*/
static const char *szTexturePointLightDiffVP =	  
	"uniform mat4 mvMatrix;"
	"uniform mat4 pMatrix;"
	"uniform vec3 vLightPos;"
	"uniform vec4 vColor;"
	"attribute vec4 vVertex;"
	"attribute vec3 vNormal;"
	"varying vec4 vFragColor;"
	"attribute vec2 vTexCoord0;"
	"varying vec2 vTex;"
	"void main(void) { "
	" mat3 mNormalMatrix;"
	" mNormalMatrix[0] = normalize(mvMatrix[0].xyz);"
	" mNormalMatrix[1] = normalize(mvMatrix[1].xyz);"
	" mNormalMatrix[2] = normalize(mvMatrix[2].xyz);"
	" vec3 vNorm = normalize(mNormalMatrix * vNormal);"
	" vec4 ecPosition;"
	" vec3 ecPosition3;"
	" ecPosition = mvMatrix * vVertex;"
	" ecPosition3 = ecPosition.xyz /ecPosition.w;"
	" vec3 vLightDir = normalize(vLightPos - ecPosition3);"
	" float fDot = max(0.0, dot(vNorm, vLightDir)); "
	" vFragColor.rgb = vColor.rgb * fDot;"
	" vFragColor.a = vColor.a;"
	" vTex = vTexCoord0;"
	" mat4 mvpMatrix;"
	" mvpMatrix = pMatrix * mvMatrix;"
	" gl_Position = mvpMatrix * vVertex; "
	"}";


static const char *szTexturePointLightDiffFP =	
#ifdef OPENGL_ES
	"precision mediump float;"
#endif
	"varying vec4 vFragColor;"
	"varying vec2 vTex;"
	"uniform sampler2D textureUnit0;"
	"void main(void) { "
	" gl_FragColor = vFragColor * texture2D(textureUnit0, vTex);"
	"}";


// GLT_SHADER_TEXTURE_POINT_LIGHT_DIFF
// Modulate texture with diffuse point light



/** 
* GLShaderManager Instance
*/
GLShaderManager* GLShaderManager::_this = nullptr;
GLShaderManager* GLShaderManager::Instance()
{
	if ( GLShaderManager::_this == nullptr ) {
		GLShaderManager::_this = new GLShaderManager();
		GLShaderManager::_this->init();
	}

	return GLShaderManager::_this;
}

GLShaderManager::GLShaderManager(void)
{
	// Set stock shader handles to 0... uninitialized
	for(unsigned int i = 0; i < GLT_SHADER_LAST; i++) {
		_shaderStock[i] = 0;
	}
}

GLShaderManager::~GLShaderManager(void)
{
	// Stock shaders is the general (are you initialized test)
	if(_shaderStock[0] != 0) {
		for (unsigned int i = 0; i < GLT_SHADER_LAST; i++) {
			glDeleteProgram(_shaderStock[i]);
		}

		// Free shader table too
		for ( const auto& x : _shaderEntryVec  ) {
			glDeleteProgram(x.shaderId);
		}
	}
}

// Initialize and load the stock shaders
bool GLShaderManager::init(void)
{
	// Be warned, going over 128 shaders may cause a hickup for a reallocation.
	_shaderEntryVec.reserve(128);

	_shaderStock[GLT_SHADER_IDENTITY]	= gltLoadShaderPairSrcWithAttributes(szIdentityShaderVP, 
		szIdentityShaderFP, 1, GLT_ATTRIBUTE_VERTEX, "vVertex");

	_shaderStock[GLT_SHADER_FLAT]	= gltLoadShaderPairSrcWithAttributes(szFlatShaderVP, 
		szFlatShaderFP, 1, GLT_ATTRIBUTE_VERTEX, "vVertex");

	_shaderStock[GLT_SHADER_SHADED] = gltLoadShaderPairSrcWithAttributes(szShadedVP, 
		szShadedFP, 2, GLT_ATTRIBUTE_VERTEX, "vVertex", GLT_ATTRIBUTE_COLOR, "vColor");

	_shaderStock[GLT_SHADER_DEFAULT_LIGHT] = gltLoadShaderPairSrcWithAttributes(szDefaultLightVP, 
		szDefaultLightFP, 2, GLT_ATTRIBUTE_VERTEX, "vVertex", GLT_ATTRIBUTE_NORMAL, "vNormal");

	_shaderStock[GLT_SHADER_POINT_LIGHT_DIFF] = gltLoadShaderPairSrcWithAttributes(szPointLightDiffVP, 
		szPointLightDiffFP, 2, GLT_ATTRIBUTE_VERTEX, "vVertex", GLT_ATTRIBUTE_NORMAL, "vNormal");

	_shaderStock[GLT_SHADER_TEXTURE_REPLACE]  = gltLoadShaderPairSrcWithAttributes(szTextureReplaceVP, 
		szTextureReplaceFP, 2, GLT_ATTRIBUTE_VERTEX, "vVertex", GLT_ATTRIBUTE_TEXTURE0, "vTexCoord0");

	_shaderStock[GLT_SHADER_TEXTURE_MODULATE] = gltLoadShaderPairSrcWithAttributes(szTextureModulateVP, 
		szTextureModulateFP, 2, GLT_ATTRIBUTE_VERTEX, "vVertex", GLT_ATTRIBUTE_TEXTURE0, "vTexCoord0");

	_shaderStock[GLT_SHADER_TEXTURE_POINT_LIGHT_DIFF] = gltLoadShaderPairSrcWithAttributes(szTexturePointLightDiffVP, 
		szTexturePointLightDiffFP, 3, GLT_ATTRIBUTE_VERTEX, "vVertex", GLT_ATTRIBUTE_NORMAL, "vNormal", GLT_ATTRIBUTE_TEXTURE0, "vTexCoord0");

	_shaderStock[GLT_SHADER_TEXTURE_RECT_REPLACE] = gltLoadShaderPairSrcWithAttributes(szTextureRectReplaceVP, 
		szTextureRectReplaceFP, 2, GLT_ATTRIBUTE_VERTEX, "vVertex", GLT_ATTRIBUTE_TEXTURE0, "vTexCoord0");

	if(_shaderStock[0] != 0) {
		return true;
	}

	return false;
}

// Use a specific stock shader, and set the appropriate uniforms
GLint GLShaderManager::useStockShader(GLT_STOCK_SHADER nShaderID, ...)
{
	// Check for out of bounds
	if(nShaderID >= GLT_SHADER_LAST) {
		return -1;
	}

	// List of uniforms
	va_list uniformList;
	va_start(uniformList, nShaderID);

	// Bind to the correct shader
	glUseProgram(_shaderStock[nShaderID]);

	// Set up the uniforms
	int	iInteger;
	M3DMatrix44f* mvpMatrix;
	M3DMatrix44f* pMatrix;
	M3DMatrix44f* mvMatrix;
	M3DVector4f*  vColor;
	M3DVector3f*  vLightPos;
	GLint iTransform, iModelMatrix, iProjMatrix, iColor, iLight, iTextureUnit;

	switch(nShaderID)
	{
	case GLT_SHADER_FLAT:			
		// Just the modelview projection matrix and the color
		iTransform = glGetUniformLocation(_shaderStock[nShaderID], "mvpMatrix");
		mvpMatrix = va_arg(uniformList, M3DMatrix44f*);
		glUniformMatrix4fv(iTransform, 1, GL_FALSE, *mvpMatrix);

		iColor = glGetUniformLocation(_shaderStock[nShaderID], "vColor");
		vColor = va_arg(uniformList, M3DVector4f*);
		glUniform4fv(iColor, 1, *vColor);
		break;

	case GLT_SHADER_TEXTURE_RECT_REPLACE:
	case GLT_SHADER_TEXTURE_REPLACE:	// Just the texture place
		iTransform = glGetUniformLocation(_shaderStock[nShaderID], "mvpMatrix");
		mvpMatrix = va_arg(uniformList, M3DMatrix44f*);
		glUniformMatrix4fv(iTransform, 1, GL_FALSE, *mvpMatrix);

		iTextureUnit = glGetUniformLocation(_shaderStock[nShaderID], "textureUnit0");
		iInteger = va_arg(uniformList, int);
		glUniform1i(iTextureUnit, iInteger);
		break;

	case GLT_SHADER_TEXTURE_MODULATE: // Multiply the texture by the geometry color
		iTransform = glGetUniformLocation(_shaderStock[nShaderID], "mvpMatrix");
		mvpMatrix = va_arg(uniformList, M3DMatrix44f*);
		glUniformMatrix4fv(iTransform, 1, GL_FALSE, *mvpMatrix);

		iColor = glGetUniformLocation(_shaderStock[nShaderID], "vColor");
		vColor = va_arg(uniformList, M3DVector4f*);
		glUniform4fv(iColor, 1, *vColor);			

		iTextureUnit = glGetUniformLocation(_shaderStock[nShaderID], "textureUnit0");
		iInteger = va_arg(uniformList, int);
		glUniform1i(iTextureUnit, iInteger);
		break;

	case GLT_SHADER_DEFAULT_LIGHT:
		iModelMatrix = glGetUniformLocation(_shaderStock[nShaderID], "mvMatrix");
		mvMatrix = va_arg(uniformList, M3DMatrix44f*);
		glUniformMatrix4fv(iModelMatrix, 1, GL_FALSE, *mvMatrix);

		iProjMatrix = glGetUniformLocation(_shaderStock[nShaderID], "pMatrix");
		pMatrix = va_arg(uniformList, M3DMatrix44f*);
		glUniformMatrix4fv(iProjMatrix, 1, GL_FALSE, *pMatrix);

		iColor = glGetUniformLocation(_shaderStock[nShaderID], "vColor");
		vColor = va_arg(uniformList, M3DVector4f*);
		glUniform4fv(iColor, 1, *vColor);
		break;

	case GLT_SHADER_POINT_LIGHT_DIFF:
		iModelMatrix = glGetUniformLocation(_shaderStock[nShaderID], "mvMatrix");
		mvMatrix = va_arg(uniformList, M3DMatrix44f*);
		glUniformMatrix4fv(iModelMatrix, 1, GL_FALSE, *mvMatrix);

		iProjMatrix = glGetUniformLocation(_shaderStock[nShaderID], "pMatrix");
		pMatrix = va_arg(uniformList, M3DMatrix44f*);
		glUniformMatrix4fv(iProjMatrix, 1, GL_FALSE, *pMatrix);

		iLight = glGetUniformLocation(_shaderStock[nShaderID], "vLightPos");
		vLightPos = va_arg(uniformList, M3DVector3f*);
		glUniform3fv(iLight, 1, *vLightPos);

		iColor = glGetUniformLocation(_shaderStock[nShaderID], "vColor");
		vColor = va_arg(uniformList, M3DVector4f*);
		glUniform4fv(iColor, 1, *vColor);
		break;			

	case GLT_SHADER_TEXTURE_POINT_LIGHT_DIFF:
		iModelMatrix = glGetUniformLocation(_shaderStock[nShaderID], "mvMatrix");
		mvMatrix = va_arg(uniformList, M3DMatrix44f*);
		glUniformMatrix4fv(iModelMatrix, 1, GL_FALSE, *mvMatrix);

		iProjMatrix = glGetUniformLocation(_shaderStock[nShaderID], "pMatrix");
		pMatrix = va_arg(uniformList, M3DMatrix44f*);
		glUniformMatrix4fv(iProjMatrix, 1, GL_FALSE, *pMatrix);

		iLight = glGetUniformLocation(_shaderStock[nShaderID], "vLightPos");
		vLightPos = va_arg(uniformList, M3DVector3f*);
		glUniform3fv(iLight, 1, *vLightPos);

		iColor = glGetUniformLocation(_shaderStock[nShaderID], "vColor");
		vColor = va_arg(uniformList, M3DVector4f*);
		glUniform4fv(iColor, 1, *vColor);

		iTextureUnit = glGetUniformLocation(_shaderStock[nShaderID], "textureUnit0");
		iInteger = va_arg(uniformList, int);
		glUniform1i(iTextureUnit, iInteger);
		break;


	case GLT_SHADER_SHADED:		// Just the modelview projection matrix. Color is an attribute
		iTransform = glGetUniformLocation(_shaderStock[nShaderID], "mvpMatrix");
		pMatrix = va_arg(uniformList, M3DMatrix44f*);
		glUniformMatrix4fv(iTransform, 1, GL_FALSE, *pMatrix);
		break;

	case GLT_SHADER_IDENTITY:	// Just the Color
		iColor = glGetUniformLocation(_shaderStock[nShaderID], "vColor");
		vColor = va_arg(uniformList, M3DVector4f*);
		glUniform4fv(iColor, 1, *vColor);
	default:
		break;
	}
	va_end(uniformList);

	return _shaderStock[nShaderID];
}

// Lookup a stock shader
GLuint GLShaderManager::GetStockShader(GLT_STOCK_SHADER nShaderID)
{
	if(nShaderID >= GLT_SHADER_LAST) {
		return 0;
	}

	return _shaderStock[nShaderID];
}

// Lookup a previously loaded shader. If szFragProg == NULL, it is assumed to be
// the same name as szVertexProg
GLuint GLShaderManager::lookupShader(const char *szVertexProg, const char *szFragProg)
{
	// Linear Search... this isn't supposed to be relied on all the time
	for (unsigned int i = 0; i < _shaderEntryVec.size(); i++) {
		if ((strncmp(szVertexProg, _shaderEntryVec[i].vertexShader, MAX_SHADER_NAME_LENGTH) == 0) &&
			(strncmp(szFragProg, _shaderEntryVec[i].fragShader, MAX_SHADER_NAME_LENGTH) == 0))
			return _shaderEntryVec[i].shaderId;
	}

	// Failed
	return 0;
}

// Load a shader pair from file. The shader pair root is added to the shader
// lookup table and can be found again if necessary with LookupShader.
GLuint GLShaderManager::loadWithFile(const char* szVertexFileName, const char* szFragFileName)
{
	ShaderLookupEntry shaderEntry;

	// Make sure it's not already loaded
	GLuint uiReturn = lookupShader(szVertexFileName, szFragFileName);
	if(uiReturn != 0) {
		return uiReturn;
	}

	// Load shader and test for fail
	shaderEntry.shaderId = gltLoadShaderPair(szVertexFileName, szFragFileName);
	if(shaderEntry.shaderId == 0) {
		return 0;
	}

	// Add to the table
	strncpy(shaderEntry.vertexShader, szVertexFileName, MAX_SHADER_NAME_LENGTH);
	strncpy(shaderEntry.fragShader, szFragFileName, MAX_SHADER_NAME_LENGTH);

	_shaderEntryVec.push_back(shaderEntry);	

	return shaderEntry.shaderId;
}

// Load shaders from source text. If the szName is NULL, just make it and return the handle
// (useful for stock shaders). Otherwize, make sure it's not already there, then add to list
GLuint GLShaderManager::LoadShaderPairSrc(const char *szName, const char *szVertexSrc, const char *szFragSrc)
{
	// Just make it and return
	if(szName == nullptr) {
		return gltLoadShaderPairString(szVertexSrc, szFragSrc);
	}

	// It has a name, check for duplicate
	GLuint uiShader = lookupShader(szName, szName);
	if(uiShader != 0) {
		return uiShader;
	}

	// Ok, make it and add to table
	ShaderLookupEntry shaderEntry;
	shaderEntry.shaderId = gltLoadShaderPairString(szVertexSrc, szFragSrc);
	if(shaderEntry.shaderId == 0) {
		return 0;	// Game over, won't compile
	}

	// Add it...
	strncpy(shaderEntry.vertexShader, szName, MAX_SHADER_NAME_LENGTH);
	strncpy(shaderEntry.fragShader, szName, MAX_SHADER_NAME_LENGTH);
	_shaderEntryVec.push_back(shaderEntry);	

	return shaderEntry.shaderId;		
}

// Load the shader file, with the supplied named attributes
GLuint GLShaderManager::LoadShaderPairWithAttributes(const char *szVertexProgFileName, 
	const char *szFragmentProgFileName, ...)
{
	// Check for duplicate
	GLuint uiShader = this->lookupShader(szVertexProgFileName, szFragmentProgFileName);
	if(uiShader != 0) {
		return uiShader;
	}

	ShaderLookupEntry shaderEntry;

	// Temporary Shader objects
	GLint testVal;

	// Create shader objects
	GLuint hVertexShader = glCreateShader(GL_VERTEX_SHADER);
	GLuint hFragmentShader = glCreateShader(GL_FRAGMENT_SHADER);

	// Load them. If fail clean up and return null
	if(gltLoadShaderWithFile(szVertexProgFileName, hVertexShader) == false) {
		glDeleteShader(hVertexShader);
		glDeleteShader(hFragmentShader);
		return 0;
	}

	if(gltLoadShaderWithFile(szFragmentProgFileName, hFragmentShader) == false) {
		glDeleteShader(hVertexShader);
		glDeleteShader(hFragmentShader);
		return 0;
	}

	// Compile them
	glCompileShader(hVertexShader);
	glCompileShader(hFragmentShader);

	// Check for errors
	glGetShaderiv(hVertexShader, GL_COMPILE_STATUS, &testVal);
	if(testVal == GL_FALSE) {
		glDeleteShader(hVertexShader);
		glDeleteShader(hFragmentShader);
		return 0;
	}

	glGetShaderiv(hFragmentShader, GL_COMPILE_STATUS, &testVal);
	if(testVal == GL_FALSE) {
		glDeleteShader(hVertexShader);
		glDeleteShader(hFragmentShader);
		return 0;
	}

	// Link them - assuming it works...
	shaderEntry.shaderId = glCreateProgram();
	glAttachShader(shaderEntry.shaderId, hVertexShader);
	glAttachShader(shaderEntry.shaderId, hFragmentShader);

	// List of attributes
	va_list attributeList;
	va_start(attributeList, szFragmentProgFileName);

	char *szNextArg;
	int iArgCount = va_arg(attributeList, int);	// Number of attributes
	for(int i = 0; i < iArgCount; i++) {
		int index = va_arg(attributeList, int);
		szNextArg = va_arg(attributeList, char*);
		glBindAttribLocation(shaderEntry.shaderId, index, szNextArg);
	}

	va_end(attributeList);

	glLinkProgram(shaderEntry.shaderId);

	// These are no longer needed
	glDeleteShader(hVertexShader);
	glDeleteShader(hFragmentShader);  

	// Make sure link worked too
	glGetProgramiv(shaderEntry.shaderId, GL_LINK_STATUS, &testVal);
	if(testVal == GL_FALSE) {
		glDeleteProgram(shaderEntry.shaderId);
		return 0;
	}

	// Add it...
	strncpy(shaderEntry.vertexShader, szVertexProgFileName, MAX_SHADER_NAME_LENGTH);
	strncpy(shaderEntry.fragShader, szFragmentProgFileName, MAX_SHADER_NAME_LENGTH);
	_shaderEntryVec.push_back(shaderEntry);	

	return shaderEntry.shaderId;		
}

// Load the shader from source, with the supplied named attributes
GLuint GLShaderManager::LoadShaderPairSrcWithAttributes(const char *szName, const char *szVertexProg, 
	const char *szFragmentProg, ...)
{
	// Check for duplicate
	GLuint uiShader = this->lookupShader(szName, szName);
	if(uiShader != 0) {
		return uiShader;
	}

	ShaderLookupEntry shaderEntry;

	// Temporary Shader objects
	GLuint hVertexShader;
	GLuint hFragmentShader;  
	GLint testVal;

	// Create shader objects
	hVertexShader = glCreateShader(GL_VERTEX_SHADER);
	hFragmentShader = glCreateShader(GL_FRAGMENT_SHADER);

	// Load them. 
	gltLoadShaderWithString(szVertexProg, hVertexShader);
	gltLoadShaderWithString(szFragmentProg, hFragmentShader);

	// Compile them
	glCompileShader(hVertexShader);
	glCompileShader(hFragmentShader);

	// Check for errors
	glGetShaderiv(hVertexShader, GL_COMPILE_STATUS, &testVal);
	if(testVal == GL_FALSE) {
		glDeleteShader(hVertexShader);
		glDeleteShader(hFragmentShader);
		return 0;
	}

	glGetShaderiv(hFragmentShader, GL_COMPILE_STATUS, &testVal);
	if(testVal == GL_FALSE)
	{
		glDeleteShader(hVertexShader);
		glDeleteShader(hFragmentShader);
		return 0;
	}

	// Link them - assuming it works...
	shaderEntry.shaderId = glCreateProgram();
	glAttachShader(shaderEntry.shaderId, hVertexShader);
	glAttachShader(shaderEntry.shaderId, hFragmentShader);

	// List of attributes
	va_list attributeList;
	va_start(attributeList, szFragmentProg);

	char *szNextArg;
	int iArgCount = va_arg(attributeList, int);	// Number of attributes
	for(int i = 0; i < iArgCount; i++) {
		int index = va_arg(attributeList, int);
		szNextArg = va_arg(attributeList, char*);
		glBindAttribLocation(shaderEntry.shaderId, index, szNextArg);
	}
	va_end(attributeList);


	glLinkProgram(shaderEntry.shaderId);

	// These are no longer needed
	glDeleteShader(hVertexShader);
	glDeleteShader(hFragmentShader);  

	// Make sure link worked too
	glGetProgramiv(shaderEntry.shaderId, GL_LINK_STATUS, &testVal);
	if(testVal == GL_FALSE) {
		glDeleteProgram(shaderEntry.shaderId);
		return 0;
	}

	// Add it...
	strncpy(shaderEntry.vertexShader, szName, MAX_SHADER_NAME_LENGTH);
	strncpy(shaderEntry.fragShader, szName, MAX_SHADER_NAME_LENGTH);
	_shaderEntryVec.push_back(shaderEntry);	

	return shaderEntry.shaderId;		
}