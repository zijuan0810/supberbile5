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
	: _numTextureUnits(0)
	, _numVerts(0)
	, _vertex(nullptr)
	, _normal(nullptr)
	, _color(nullptr)
	, _texCoords(nullptr)
	, _vertexBuf(0)
	, _normalBuf(0)
	, _colorBuf(0)
	, _vertexArrayObject(0)
	, _bBatchDone(false)
	, _numVertsBuilding(0)
	, _texCoordVec(nullptr)
{
}

GLBatch::~GLBatch(void)
{
	// Vertex buffer objects
	if(_vertexBuf != 0)
		glDeleteBuffers(1, &_vertexBuf);

	if(_normalBuf != 0)
		glDeleteBuffers(1, &_normalBuf);

	if(_colorBuf != 0)
		glDeleteBuffers(1, &_colorBuf);

	for(unsigned int i = 0; i < _numTextureUnits; i++)
		glDeleteBuffers(1, &_texCoordVec[i]);

#ifndef OPENGL_ES
	glDeleteVertexArrays(1, &_vertexArrayObject);
#endif

	delete [] _texCoordVec;
	delete [] _texCoords;
}


// Start the primitive batch.
void GLBatch::Begin(GLenum primitive, GLuint nVerts, GLuint nTextureUnits)
{
	_primitiveType = primitive;
	_numVerts = nVerts;

	// Save and Limit to four texture units
	_numTextureUnits = min(nTextureUnits, 4);
	if(_numTextureUnits != 0) {
		_texCoordVec = new GLuint[_numTextureUnits];

		// An array of pointers to texture coordinate arrays
		_texCoords = new M3DVector2f*[_numTextureUnits];
		for(unsigned int i = 0; i < _numTextureUnits; i++) {
			_texCoordVec[i] = 0;
			_texCoords[i] = nullptr;
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
	if(_vertexBuf == 0) {
		glGenBuffers(1, &_vertexBuf);
		glBindBuffer(GL_ARRAY_BUFFER, _vertexBuf);
		glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 3 * _numVerts, vVerts, GL_DYNAMIC_DRAW);
	}
	else { 
		// Just bind to existing object
		glBindBuffer(GL_ARRAY_BUFFER, _vertexBuf);

		// Copy the data in
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(GLfloat) * 3 * _numVerts, vVerts);
		_vertex = nullptr;
	}
}

// Block copy in normal data
void GLBatch::CopyNormalDataf(M3DVector3f *vNorms) 
{
	// First time, create the buffer object, allocate the space
	if(_normalBuf == 0) {
		glGenBuffers(1, &_normalBuf);
		glBindBuffer(GL_ARRAY_BUFFER, _normalBuf);
		glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 3 * _numVerts, vNorms, GL_DYNAMIC_DRAW);
	}
	else {	// Just bind to existing object
		glBindBuffer(GL_ARRAY_BUFFER, _normalBuf);

		// Copy the data in
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(GLfloat) * 3 * _numVerts, vNorms);
		_normal = nullptr;
	}
}

void GLBatch::CopyColorData4f(M3DVector4f *vColors) 
{
	// First time, create the buffer object, allocate the space
	if(_colorBuf == 0) {
		glGenBuffers(1, &_colorBuf);
		glBindBuffer(GL_ARRAY_BUFFER, _colorBuf);
		glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 4 * _numVerts, vColors, GL_DYNAMIC_DRAW);
	}
	else {	// Just bind to existing object
		glBindBuffer(GL_ARRAY_BUFFER, _colorBuf);

		// Copy the data in
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(GLfloat) * 4 * _numVerts, vColors);
		_color = nullptr;
	}
}

void GLBatch::CopyTexCoordData2f(M3DVector2f *vTexCoords, GLuint uiTextureLayer) 
{
	// First time, create the buffer object, allocate the space
	if(_texCoordVec[uiTextureLayer] == 0) {
		glGenBuffers(1, &_texCoordVec[uiTextureLayer]);
		glBindBuffer(GL_ARRAY_BUFFER, _texCoordVec[uiTextureLayer]);
		glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 2 * _numVerts, vTexCoords, GL_DYNAMIC_DRAW);
	}
	else {	// Just bind to existing object
		glBindBuffer(GL_ARRAY_BUFFER, _texCoordVec[uiTextureLayer]);

		// Copy the data in
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(GLfloat) * 2 * _numVerts, vTexCoords);
		_texCoords[uiTextureLayer] = nullptr;
	}
}

// Bind everything up in a little package
void GLBatch::End(void)
{
#ifndef OPENGL_ES
	// Check to see if items have been added one at a time
	if(_vertex != nullptr) {
		glBindBuffer(GL_ARRAY_BUFFER, _vertexBuf);
		glUnmapBuffer(GL_ARRAY_BUFFER);
		_vertex = nullptr;
	}

	if(_color != nullptr) {
		glBindBuffer(GL_ARRAY_BUFFER, _colorBuf);
		glUnmapBuffer(GL_ARRAY_BUFFER);
		_color = nullptr; 
	}

	if(_normal != nullptr) {
		glBindBuffer(GL_ARRAY_BUFFER, _normalBuf);
		glUnmapBuffer(GL_ARRAY_BUFFER);
		_normal = nullptr;
	}

	for(unsigned int i = 0; i < _numTextureUnits; i++) {
		if(_texCoords[i] != nullptr) {
			glBindBuffer(GL_ARRAY_BUFFER, _texCoordVec[i]);
			glUnmapBuffer(GL_ARRAY_BUFFER);
			_texCoords[i] = nullptr;
		}
	}

	// Set up the vertex array object
	glBindVertexArray(_vertexArrayObject);
#endif

	if(_vertexBuf !=0) {
		glEnableVertexAttribArray(GLT_ATTRIBUTE_VERTEX);
		glBindBuffer(GL_ARRAY_BUFFER, _vertexBuf);
		glVertexAttribPointer(GLT_ATTRIBUTE_VERTEX, 3, GL_FLOAT, GL_FALSE, 0, 0);
	}

	if(_colorBuf != 0) {
		glEnableVertexAttribArray(GLT_ATTRIBUTE_COLOR);
		glBindBuffer(GL_ARRAY_BUFFER, _colorBuf);
		glVertexAttribPointer(GLT_ATTRIBUTE_COLOR, 4, GL_FLOAT, GL_FALSE, 0, 0);
	}

	if(_normalBuf != 0) {
		glEnableVertexAttribArray(GLT_ATTRIBUTE_NORMAL);
		glBindBuffer(GL_ARRAY_BUFFER, _normalBuf);
		glVertexAttribPointer(GLT_ATTRIBUTE_NORMAL, 3, GL_FLOAT, GL_FALSE, 0, 0);
	}

	// How many texture units
	for(unsigned int i = 0; i < _numTextureUnits; i++) {
		if(_texCoordVec[i] != 0) {
			glEnableVertexAttribArray(GLT_ATTRIBUTE_TEXTURE0 + i), glBindBuffer(GL_ARRAY_BUFFER, _texCoordVec[i]);
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
	_numVertsBuilding = 0;
}


void GLBatch::Vertex3f(GLfloat x, GLfloat y, GLfloat z)
{
	// First see if the vertex array buffer has been created...
	if(_vertexBuf == 0) {	// Nope, we need to create it
		glGenBuffers(1, &_vertexBuf);
		glBindBuffer(GL_ARRAY_BUFFER, _vertexBuf);
		glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 3 * _numVerts, nullptr, GL_DYNAMIC_DRAW);
	}

	// Now see if it's already mapped, if not, map it
	if(_vertex == nullptr) {
		glBindBuffer(GL_ARRAY_BUFFER, _vertexBuf);
		_vertex = (M3DVector3f *)glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
	}

	// Ignore if we go past the end, keeps things from blowing up
	if(_numVertsBuilding >= _numVerts) {
		return;
	}

	// Copy it in...
	_vertex[_numVertsBuilding][0] = x;
	_vertex[_numVertsBuilding][1] = y;
	_vertex[_numVertsBuilding][2] = z;
	_numVertsBuilding++;
}

void GLBatch::Vertex3fv(M3DVector3f vVertex)
{
	// First see if the vertex array buffer has been created...
	if(_vertexBuf == 0) {	// Nope, we need to create it
		glGenBuffers(1, &_vertexBuf);
		glBindBuffer(GL_ARRAY_BUFFER, _vertexBuf);
		glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 3 * _numVerts, nullptr, GL_DYNAMIC_DRAW);
	}

	// Now see if it's already mapped, if not, map it
	if(_vertex == nullptr) {
		glBindBuffer(GL_ARRAY_BUFFER, _vertexBuf);
		_vertex = (M3DVector3f *)glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
	}

	// Ignore if we go past the end, keeps things from blowing up
	if(_numVertsBuilding >= _numVerts)
		return;

	// Copy it in...
	memcpy(_vertex[_numVertsBuilding], vVertex, sizeof(M3DVector3f));
	_numVertsBuilding++;
}

void GLBatch::Normal3f(GLfloat x, GLfloat y, GLfloat z)
{
	// First see if the vertex array buffer has been created...
	if(_normalBuf == 0) {	// Nope, we need to create it
		glGenBuffers(1, &_normalBuf);
		glBindBuffer(GL_ARRAY_BUFFER, _normalBuf);
		glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 3 * _numVerts, nullptr, GL_DYNAMIC_DRAW);
	}

	// Now see if it's already mapped, if not, map it
	if(_normal == nullptr) {
		glBindBuffer(GL_ARRAY_BUFFER, _normalBuf);
		_normal = (M3DVector3f *)glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
	}

	// Ignore if we go past the end, keeps things from blowing up
	if(_numVertsBuilding >= _numVerts) {
		return;
	}

	// Copy it in...
	_normal[_numVertsBuilding][0] = x;
	_normal[_numVertsBuilding][1] = y;
	_normal[_numVertsBuilding][2] = z;
}

// Ditto above
void GLBatch::Normal3fv(M3DVector3f vNormal)
{
	// First see if the vertex array buffer has been created...
	if(_normalBuf == 0) {	// Nope, we need to create it
		glGenBuffers(1, &_normalBuf);
		glBindBuffer(GL_ARRAY_BUFFER, _normalBuf);
		glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 3 * _numVerts, nullptr, GL_DYNAMIC_DRAW);
	}

	// Now see if it's already mapped, if not, map it
	if(_normal == nullptr) {
		glBindBuffer(GL_ARRAY_BUFFER, _normalBuf);
		_normal = (M3DVector3f *)glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
	}

	// Ignore if we go past the end, keeps things from blowing up
	if(_numVertsBuilding >= _numVerts)
		return;

	// Copy it in...
	memcpy(_normal[_numVertsBuilding], vNormal, sizeof(M3DVector3f));
}


void GLBatch::Color4f(GLfloat r, GLfloat g, GLfloat b, GLfloat a)
{
	// First see if the vertex array buffer has been created...
	if(_colorBuf == 0) {	// Nope, we need to create it
		glGenBuffers(1, &_colorBuf);
		glBindBuffer(GL_ARRAY_BUFFER, _colorBuf);
		glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 4 * _numVerts, nullptr, GL_DYNAMIC_DRAW);
	}

	// Now see if it's already mapped, if not, map it
	if(_color == nullptr) {
		glBindBuffer(GL_ARRAY_BUFFER, _colorBuf);
		_color = (M3DVector4f*)glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
	}

	// Ignore if we go past the end, keeps things from blowing up
	if(_numVertsBuilding >= _numVerts)
		return;

	// Copy it in...
	_color[_numVertsBuilding][0] = r;
	_color[_numVertsBuilding][1] = g;
	_color[_numVertsBuilding][2] = b;
	_color[_numVertsBuilding][3] = a;
}

void GLBatch::Color4fv(M3DVector4f vColor)
{
	// First see if the vertex array buffer has been created...
	if(_colorBuf == 0) {	// Nope, we need to create it
		glGenBuffers(1, &_colorBuf);
		glBindBuffer(GL_ARRAY_BUFFER, _colorBuf);
		glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 4 * _numVerts, nullptr, GL_DYNAMIC_DRAW);
	}

	// Now see if it's already mapped, if not, map it
	if(_color == nullptr) {
		glBindBuffer(GL_ARRAY_BUFFER, _colorBuf);
		_color = (M3DVector4f *)glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
	}

	// Ignore if we go past the end, keeps things from blowing up
	if(_numVertsBuilding >= _numVerts)
		return;

	// Copy it in...
	memcpy(_color[_numVertsBuilding], vColor, sizeof(M3DVector4f));
}

/**
* Unlike normal OpenGL immediate mode, you must specify a texture coord
* per vertex or you will get junk...
*/
void GLBatch::MultiTexCoord2f(GLuint texture, GLclampf s, GLclampf t)
{
	// First see if the vertex array buffer has been created...
	if(_texCoordVec[texture] == 0) {	// Nope, we need to create it
		glGenBuffers(1, &_texCoordVec[texture]);
		glBindBuffer(GL_ARRAY_BUFFER, _texCoordVec[texture]);
		glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 2 * _numVerts, nullptr, GL_DYNAMIC_DRAW);
	}

	// Now see if it's already mapped, if not, map it
	if(_texCoords[texture] == nullptr) {
		glBindBuffer(GL_ARRAY_BUFFER, _texCoordVec[texture]);
		_texCoords[texture] = (M3DVector2f *)glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
	}

	// Ignore if we go past the end, keeps things from blowing up
	if(_numVertsBuilding >= _numVerts) {
		return;
	}

	// Copy it in...
	_texCoords[texture][_numVertsBuilding][0] = s;
	_texCoords[texture][_numVertsBuilding][1] = t;
}

// Ditto above  
void GLBatch::MultiTexCoord2fv(GLuint texture, M3DVector2f vTexCoord)
{	
	// First see if the vertex array buffer has been created...
	if(_texCoordVec[texture] == 0) {	// Nope, we need to create it
		glGenBuffers(1, &_texCoordVec[texture]);
		glBindBuffer(GL_ARRAY_BUFFER, _texCoordVec[texture]);
		glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 2 * _numVerts, nullptr, GL_DYNAMIC_DRAW);
	}

	// Now see if it's already mapped, if not, map it
	if(_texCoords[texture] == nullptr) {
		glBindBuffer(GL_ARRAY_BUFFER, _texCoordVec[texture]);
		_texCoords[texture] = (M3DVector2f *)glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
	}

	// Ignore if we go past the end, keeps things from blowing up
	if(_numVertsBuilding >= _numVerts)
		return;

	// Copy it in...
	memcpy(_texCoords[texture], vTexCoord, sizeof(M3DVector2f));
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
	if(_vertexBuf !=0) {
		glEnableVertexAttribArray(GLT_ATTRIBUTE_VERTEX);
		glBindBuffer(GL_ARRAY_BUFFER, _vertexBuf);
		glVertexAttribPointer(GLT_ATTRIBUTE_VERTEX, 3, GL_FLOAT, GL_FALSE, 0, 0);
	}

	if(_colorBuf != 0) {
		glEnableVertexAttribArray(GLT_ATTRIBUTE_COLOR);
		glBindBuffer(GL_ARRAY_BUFFER, _colorBuf);
		glVertexAttribPointer(GLT_ATTRIBUTE_COLOR, 4, GL_FLOAT, GL_FALSE, 0, 0);
	}

	if(_normalBuf != 0) {
		glEnableVertexAttribArray(GLT_ATTRIBUTE_NORMAL);
		glBindBuffer(GL_ARRAY_BUFFER, _normalBuf);
		glVertexAttribPointer(GLT_ATTRIBUTE_NORMAL, 3, GL_FLOAT, GL_FALSE, 0, 0);
	}

	// How many texture units
	for(unsigned int i = 0; i < _numTextureUnits; i++)
		if(_texCoordVec[i] != 0) {
			glEnableVertexAttribArray(GLT_ATTRIBUTE_TEXTURE0 + i),
				glBindBuffer(GL_ARRAY_BUFFER, _texCoordVec[i]);
			glVertexAttribPointer(GLT_ATTRIBUTE_TEXTURE0 + i, 2, GL_FLOAT, GL_FALSE, 0, 0);
		}
#endif
		glDrawArrays(_primitiveType, 0, _numVerts);

#ifndef OPENGL_ES
		glBindVertexArray(0);
#else
		glDisableVertexAttribArray(GLT_ATTRIBUTE_VERTEX);
		glDisableVertexAttribArray(GLT_ATTRIBUTE_NORMAL);
		glDisableVertexAttribArray(GLT_ATTRIBUTE_COLOR);

		for(unsigned int i = 0; i < _numTextureUnits; i++) {
			if(_texCoordVec[i] != 0) {
				glDisableVertexAttribArray(GLT_ATTRIBUTE_TEXTURE0 + i);
			}
		}

#endif
}

#endif
