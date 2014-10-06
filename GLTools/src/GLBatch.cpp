#ifndef __GLT_BATCH
#define __GLT_BATCH

#include "GLBatch.h"
#include "GLShaderManager.h"


//////////////////////// TEMPORARY TEMPORARY TEMPORARY - On SnowLeopard this is suppored, but GLEW doens't hook up properly
//////////////////////// Fixed probably in 10.6.3
#ifdef __APPLE__
#define glGenVertexArrays glGenVertexArraysAPPLE
#define glDeleteVertexArrays  glDeleteVertexArraysAPPLE
#define glBindVertexArray	glBindVertexArrayAPPLE
#endif

/////////////////////// OpenGL ES support on iPhone/iPad
#ifdef OPENGL_ES
#define GL_WRITE_ONLY   GL_WRITE_ONLY_OES
#define glMapBuffer     glMapBufferOES
#define glUnmapBuffer   glUnmapBufferOES
#endif

GLBatch::GLBatch(void)
	: _nNumTextureUnits(0)
	, _nNumVerts(0)
	, _pVerts(nullptr)
	, _pNormals(nullptr)
	, _pColors(nullptr)
	, _pTexCoords(nullptr)
	, _uiVertexArray(0)
	, _uiNormalArray(0)
	, _uiColorArray(0)
	, _vertexArrayObject(0)
	, _bBatchDone(false)
	, _nVertsBuilding(0)
	, _uiTextureCoordArray(nullptr)
{
}

GLBatch::~GLBatch(void)
{
	// Vertex buffer objects
	if(_uiVertexArray != 0)
		glDeleteBuffers(1, &_uiVertexArray);

	if(_uiNormalArray != 0)
		glDeleteBuffers(1, &_uiNormalArray);

	if(_uiColorArray != 0)
		glDeleteBuffers(1, &_uiColorArray);

	for(unsigned int i = 0; i < _nNumTextureUnits; i++)
		glDeleteBuffers(1, &_uiTextureCoordArray[i]);

#ifndef OPENGL_ES
	glDeleteVertexArrays(1, &_vertexArrayObject);
#endif

	delete [] _uiTextureCoordArray;
	delete [] _pTexCoords;
}


// Start the primitive batch.
void GLBatch::Begin(GLenum primitive, GLuint nVerts, GLuint nTextureUnits)
{
	_primitiveType = primitive;
	_nNumVerts = nVerts;

	// Save and Limit to four texture units
	_nNumTextureUnits = min(nTextureUnits, 4);
	if(_nNumTextureUnits != 0) {
		_uiTextureCoordArray = new GLuint[_nNumTextureUnits];

		// An array of pointers to texture coordinate arrays
		_pTexCoords = new M3DVector2f*[_nNumTextureUnits];
		for(unsigned int i = 0; i < _nNumTextureUnits; i++) {
			_uiTextureCoordArray[i] = 0;
			_pTexCoords[i] = nullptr;
		}
	}

	// Vertex Array object for this Array
#ifndef OPENGL_ES
	glGenVertexArrays(1, &_vertexArrayObject);
	glBindVertexArray(_vertexArrayObject);
#endif
}


// Block Copy in vertex data
void GLBatch::CopyVertexData3f(M3DVector3f *vVerts) 
{
	// First time, create the buffer object, allocate the space
	if(_uiVertexArray == 0) {
		glGenBuffers(1, &_uiVertexArray);
		glBindBuffer(GL_ARRAY_BUFFER, _uiVertexArray);
		glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 3 * _nNumVerts, vVerts, GL_DYNAMIC_DRAW);
	}
	else { 
		// Just bind to existing object
		glBindBuffer(GL_ARRAY_BUFFER, _uiVertexArray);

		// Copy the data in
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(GLfloat) * 3 * _nNumVerts, vVerts);
		_pVerts = nullptr;
	}
}

// Block copy in normal data
void GLBatch::CopyNormalDataf(M3DVector3f *vNorms) 
{
	// First time, create the buffer object, allocate the space
	if(_uiNormalArray == 0) {
		glGenBuffers(1, &_uiNormalArray);
		glBindBuffer(GL_ARRAY_BUFFER, _uiNormalArray);
		glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 3 * _nNumVerts, vNorms, GL_DYNAMIC_DRAW);
	}
	else {	// Just bind to existing object
		glBindBuffer(GL_ARRAY_BUFFER, _uiNormalArray);

		// Copy the data in
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(GLfloat) * 3 * _nNumVerts, vNorms);
		_pNormals = nullptr;
	}
}

void GLBatch::CopyColorData4f(M3DVector4f *vColors) 
{
	// First time, create the buffer object, allocate the space
	if(_uiColorArray == 0) {
		glGenBuffers(1, &_uiColorArray);
		glBindBuffer(GL_ARRAY_BUFFER, _uiColorArray);
		glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 4 * _nNumVerts, vColors, GL_DYNAMIC_DRAW);
	}
	else {	// Just bind to existing object
		glBindBuffer(GL_ARRAY_BUFFER, _uiColorArray);

		// Copy the data in
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(GLfloat) * 4 * _nNumVerts, vColors);
		_pColors = nullptr;
	}
}

void GLBatch::CopyTexCoordData2f(M3DVector2f *vTexCoords, GLuint uiTextureLayer) 
{
	// First time, create the buffer object, allocate the space
	if(_uiTextureCoordArray[uiTextureLayer] == 0) {
		glGenBuffers(1, &_uiTextureCoordArray[uiTextureLayer]);
		glBindBuffer(GL_ARRAY_BUFFER, _uiTextureCoordArray[uiTextureLayer]);
		glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 2 * _nNumVerts, vTexCoords, GL_DYNAMIC_DRAW);
	}
	else {	// Just bind to existing object
		glBindBuffer(GL_ARRAY_BUFFER, _uiTextureCoordArray[uiTextureLayer]);

		// Copy the data in
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(GLfloat) * 2 * _nNumVerts, vTexCoords);
		_pTexCoords[uiTextureLayer] = nullptr;
	}
}

// Bind everything up in a little package
void GLBatch::End(void)
{
#ifndef OPENGL_ES
	// Check to see if items have been added one at a time
	if(_pVerts != nullptr) {
		glBindBuffer(GL_ARRAY_BUFFER, _uiVertexArray);
		glUnmapBuffer(GL_ARRAY_BUFFER);
		_pVerts = nullptr;
	}

	if(_pColors != nullptr) {
		glBindBuffer(GL_ARRAY_BUFFER, _uiColorArray);
		glUnmapBuffer(GL_ARRAY_BUFFER);
		_pColors = nullptr; 
	}

	if(_pNormals != nullptr) {
		glBindBuffer(GL_ARRAY_BUFFER, _uiNormalArray);
		glUnmapBuffer(GL_ARRAY_BUFFER);
		_pNormals = nullptr;
	}

	for(unsigned int i = 0; i < _nNumTextureUnits; i++) {
		if(_pTexCoords[i] != nullptr) {
			glBindBuffer(GL_ARRAY_BUFFER, _uiTextureCoordArray[i]);
			glUnmapBuffer(GL_ARRAY_BUFFER);
			_pTexCoords[i] = nullptr;
		}
	}

	// Set up the vertex array object
	glBindVertexArray(_vertexArrayObject);
#endif

	if(_uiVertexArray !=0) {
		glEnableVertexAttribArray(GLT_ATTRIBUTE_VERTEX);
		glBindBuffer(GL_ARRAY_BUFFER, _uiVertexArray);
		glVertexAttribPointer(GLT_ATTRIBUTE_VERTEX, 3, GL_FLOAT, GL_FALSE, 0, 0);
	}

	if(_uiColorArray != 0) {
		glEnableVertexAttribArray(GLT_ATTRIBUTE_COLOR);
		glBindBuffer(GL_ARRAY_BUFFER, _uiColorArray);
		glVertexAttribPointer(GLT_ATTRIBUTE_COLOR, 4, GL_FLOAT, GL_FALSE, 0, 0);
	}

	if(_uiNormalArray != 0) {
		glEnableVertexAttribArray(GLT_ATTRIBUTE_NORMAL);
		glBindBuffer(GL_ARRAY_BUFFER, _uiNormalArray);
		glVertexAttribPointer(GLT_ATTRIBUTE_NORMAL, 3, GL_FLOAT, GL_FALSE, 0, 0);
	}

	// How many texture units
	for(unsigned int i = 0; i < _nNumTextureUnits; i++) {
		if(_uiTextureCoordArray[i] != 0) {
			glEnableVertexAttribArray(GLT_ATTRIBUTE_TEXTURE0 + i), glBindBuffer(GL_ARRAY_BUFFER, _uiTextureCoordArray[i]);
			glVertexAttribPointer(GLT_ATTRIBUTE_TEXTURE0 + i, 2, GL_FLOAT, GL_FALSE, 0, 0);
		}
	}

	_bBatchDone = true;

#ifndef OPENGL_ES
	glBindVertexArray(0);
#endif
}


// Just start over. No reallocations, etc.
void GLBatch::Reset(void)
{
	_bBatchDone = false;
	_nVertsBuilding = 0;
}


void GLBatch::Vertex3f(GLfloat x, GLfloat y, GLfloat z)
{
	// First see if the vertex array buffer has been created...
	if(_uiVertexArray == 0) {	// Nope, we need to create it
		glGenBuffers(1, &_uiVertexArray);
		glBindBuffer(GL_ARRAY_BUFFER, _uiVertexArray);
		glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 3 * _nNumVerts, nullptr, GL_DYNAMIC_DRAW);
	}

	// Now see if it's already mapped, if not, map it
	if(_pVerts == nullptr) {
		glBindBuffer(GL_ARRAY_BUFFER, _uiVertexArray);
		_pVerts = (M3DVector3f *)glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
	}

	// Ignore if we go past the end, keeps things from blowing up
	if(_nVertsBuilding >= _nNumVerts) {
		return;
	}

	// Copy it in...
	_pVerts[_nVertsBuilding][0] = x;
	_pVerts[_nVertsBuilding][1] = y;
	_pVerts[_nVertsBuilding][2] = z;
	_nVertsBuilding++;
}

void GLBatch::Vertex3fv(M3DVector3f vVertex)
{
	// First see if the vertex array buffer has been created...
	if(_uiVertexArray == 0) {	// Nope, we need to create it
		glGenBuffers(1, &_uiVertexArray);
		glBindBuffer(GL_ARRAY_BUFFER, _uiVertexArray);
		glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 3 * _nNumVerts, nullptr, GL_DYNAMIC_DRAW);
	}

	// Now see if it's already mapped, if not, map it
	if(_pVerts == nullptr) {
		glBindBuffer(GL_ARRAY_BUFFER, _uiVertexArray);
		_pVerts = (M3DVector3f *)glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
	}

	// Ignore if we go past the end, keeps things from blowing up
	if(_nVertsBuilding >= _nNumVerts)
		return;

	// Copy it in...
	memcpy(_pVerts[_nVertsBuilding], vVertex, sizeof(M3DVector3f));
	_nVertsBuilding++;
}

void GLBatch::Normal3f(GLfloat x, GLfloat y, GLfloat z)
{
	// First see if the vertex array buffer has been created...
	if(_uiNormalArray == 0) {	// Nope, we need to create it
		glGenBuffers(1, &_uiNormalArray);
		glBindBuffer(GL_ARRAY_BUFFER, _uiNormalArray);
		glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 3 * _nNumVerts, nullptr, GL_DYNAMIC_DRAW);
	}

	// Now see if it's already mapped, if not, map it
	if(_pNormals == nullptr) {
		glBindBuffer(GL_ARRAY_BUFFER, _uiNormalArray);
		_pNormals = (M3DVector3f *)glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
	}

	// Ignore if we go past the end, keeps things from blowing up
	if(_nVertsBuilding >= _nNumVerts) {
		return;
	}

	// Copy it in...
	_pNormals[_nVertsBuilding][0] = x;
	_pNormals[_nVertsBuilding][1] = y;
	_pNormals[_nVertsBuilding][2] = z;
}

// Ditto above
void GLBatch::Normal3fv(M3DVector3f vNormal)
{
	// First see if the vertex array buffer has been created...
	if(_uiNormalArray == 0) {	// Nope, we need to create it
		glGenBuffers(1, &_uiNormalArray);
		glBindBuffer(GL_ARRAY_BUFFER, _uiNormalArray);
		glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 3 * _nNumVerts, nullptr, GL_DYNAMIC_DRAW);
	}

	// Now see if it's already mapped, if not, map it
	if(_pNormals == nullptr) {
		glBindBuffer(GL_ARRAY_BUFFER, _uiNormalArray);
		_pNormals = (M3DVector3f *)glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
	}

	// Ignore if we go past the end, keeps things from blowing up
	if(_nVertsBuilding >= _nNumVerts)
		return;

	// Copy it in...
	memcpy(_pNormals[_nVertsBuilding], vNormal, sizeof(M3DVector3f));
}


void GLBatch::Color4f(GLfloat r, GLfloat g, GLfloat b, GLfloat a)
{
	// First see if the vertex array buffer has been created...
	if(_uiColorArray == 0) {	// Nope, we need to create it
		glGenBuffers(1, &_uiColorArray);
		glBindBuffer(GL_ARRAY_BUFFER, _uiColorArray);
		glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 4 * _nNumVerts, nullptr, GL_DYNAMIC_DRAW);
	}

	// Now see if it's already mapped, if not, map it
	if(_pColors == nullptr) {
		glBindBuffer(GL_ARRAY_BUFFER, _uiColorArray);
		_pColors = (M3DVector4f *)glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
	}

	// Ignore if we go past the end, keeps things from blowing up
	if(_nVertsBuilding >= _nNumVerts)
		return;

	// Copy it in...
	_pColors[_nVertsBuilding][0] = r;
	_pColors[_nVertsBuilding][1] = g;
	_pColors[_nVertsBuilding][2] = b;
	_pColors[_nVertsBuilding][3] = a;
}

void GLBatch::Color4fv(M3DVector4f vColor)
{
	// First see if the vertex array buffer has been created...
	if(_uiColorArray == 0) {	// Nope, we need to create it
		glGenBuffers(1, &_uiColorArray);
		glBindBuffer(GL_ARRAY_BUFFER, _uiColorArray);
		glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 4 * _nNumVerts, nullptr, GL_DYNAMIC_DRAW);
	}

	// Now see if it's already mapped, if not, map it
	if(_pColors == nullptr) {
		glBindBuffer(GL_ARRAY_BUFFER, _uiColorArray);
		_pColors = (M3DVector4f *)glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
	}

	// Ignore if we go past the end, keeps things from blowing up
	if(_nVertsBuilding >= _nNumVerts)
		return;

	// Copy it in...
	memcpy(_pColors[_nVertsBuilding], vColor, sizeof(M3DVector4f));
}

/**
* Unlike normal OpenGL immediate mode, you must specify a texture coord
* per vertex or you will get junk...
*/
void GLBatch::MultiTexCoord2f(GLuint texture, GLclampf s, GLclampf t)
{
	// First see if the vertex array buffer has been created...
	if(_uiTextureCoordArray[texture] == 0) {	// Nope, we need to create it
		glGenBuffers(1, &_uiTextureCoordArray[texture]);
		glBindBuffer(GL_ARRAY_BUFFER, _uiTextureCoordArray[texture]);
		glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 2 * _nNumVerts, nullptr, GL_DYNAMIC_DRAW);
	}

	// Now see if it's already mapped, if not, map it
	if(_pTexCoords[texture] == nullptr) {
		glBindBuffer(GL_ARRAY_BUFFER, _uiTextureCoordArray[texture]);
		_pTexCoords[texture] = (M3DVector2f *)glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
	}

	// Ignore if we go past the end, keeps things from blowing up
	if(_nVertsBuilding >= _nNumVerts) {
		return;
	}

	// Copy it in...
	_pTexCoords[texture][_nVertsBuilding][0] = s;
	_pTexCoords[texture][_nVertsBuilding][1] = t;
}

// Ditto above  
void GLBatch::MultiTexCoord2fv(GLuint texture, M3DVector2f vTexCoord)
{	
	// First see if the vertex array buffer has been created...
	if(_uiTextureCoordArray[texture] == 0) {	// Nope, we need to create it
		glGenBuffers(1, &_uiTextureCoordArray[texture]);
		glBindBuffer(GL_ARRAY_BUFFER, _uiTextureCoordArray[texture]);
		glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 2 * _nNumVerts, nullptr, GL_DYNAMIC_DRAW);
	}

	// Now see if it's already mapped, if not, map it
	if(_pTexCoords[texture] == nullptr) {
		glBindBuffer(GL_ARRAY_BUFFER, _uiTextureCoordArray[texture]);
		_pTexCoords[texture] = (M3DVector2f *)glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
	}

	// Ignore if we go past the end, keeps things from blowing up
	if(_nVertsBuilding >= _nNumVerts)
		return;

	// Copy it in...
	memcpy(_pTexCoords[texture], vTexCoord, sizeof(M3DVector2f));
}


void GLBatch::Draw(void)
{
	if(!_bBatchDone) {
		return;
	}

#ifndef OPENGL_ES
	// Set up the vertex array object
	glBindVertexArray(_vertexArrayObject);
#else
	if(_uiVertexArray !=0) {
		glEnableVertexAttribArray(GLT_ATTRIBUTE_VERTEX);
		glBindBuffer(GL_ARRAY_BUFFER, _uiVertexArray);
		glVertexAttribPointer(GLT_ATTRIBUTE_VERTEX, 3, GL_FLOAT, GL_FALSE, 0, 0);
	}

	if(_uiColorArray != 0) {
		glEnableVertexAttribArray(GLT_ATTRIBUTE_COLOR);
		glBindBuffer(GL_ARRAY_BUFFER, _uiColorArray);
		glVertexAttribPointer(GLT_ATTRIBUTE_COLOR, 4, GL_FLOAT, GL_FALSE, 0, 0);
	}

	if(_uiNormalArray != 0) {
		glEnableVertexAttribArray(GLT_ATTRIBUTE_NORMAL);
		glBindBuffer(GL_ARRAY_BUFFER, _uiNormalArray);
		glVertexAttribPointer(GLT_ATTRIBUTE_NORMAL, 3, GL_FLOAT, GL_FALSE, 0, 0);
	}

	// How many texture units
	for(unsigned int i = 0; i < _nNumTextureUnits; i++)
		if(_uiTextureCoordArray[i] != 0) {
			glEnableVertexAttribArray(GLT_ATTRIBUTE_TEXTURE0 + i),
				glBindBuffer(GL_ARRAY_BUFFER, _uiTextureCoordArray[i]);
			glVertexAttribPointer(GLT_ATTRIBUTE_TEXTURE0 + i, 2, GL_FLOAT, GL_FALSE, 0, 0);
		}
#endif
		glDrawArrays(_primitiveType, 0, _nNumVerts);

#ifndef OPENGL_ES
		glBindVertexArray(0);
#else
		glDisableVertexAttribArray(GLT_ATTRIBUTE_VERTEX);
		glDisableVertexAttribArray(GLT_ATTRIBUTE_NORMAL);
		glDisableVertexAttribArray(GLT_ATTRIBUTE_COLOR);

		for(unsigned int i = 0; i < _nNumTextureUnits; i++) {
			if(_uiTextureCoordArray[i] != 0) {
				glDisableVertexAttribArray(GLT_ATTRIBUTE_TEXTURE0 + i);
			}
		}

#endif
}

#endif
