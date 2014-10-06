// GLMatrixStack.h
// Matrix stack functionality
/*
Copyright (c) 2009, Richard S. Wright Jr.
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, 
are permitted provided that the following conditions are met:

Redistributions of source code must retain the above copyright notice, this list 
of conditions and the following disclaimer.

Redistributions in binary form must reproduce the above copyright notice, this list 
of conditions and the following disclaimer in the documentation and/or other 
materials provided with the distribution.

Neither the name of Richard S. Wright Jr. nor the names of other contributors may be used 
to endorse or promote products derived from this software without specific prior 
written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY 
EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES 
OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT 
SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, 
INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED 
TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR 
BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN 
CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN 
ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef __GLT_MATRIX_STACK
#define __GLT_MATRIX_STACK

#include <GLTools.h>
#include <math3d.h>
#include <GLFrame.h>

enum GLT_STACK_ERROR { 
	GLT_STACK_NOERROR	= 0, 
	GLT_STACK_OVERFLOW	= 1, 
	GLT_STACK_UNDERFLOW = 2
}; 

class GLMatrixStack
{
public:
	GLMatrixStack(int iStackDepth = 64) {
		_stackDepth = iStackDepth;
		_pStack = new M3DMatrix44f[iStackDepth];
		_stackIndex = 0;
		m3dLoadIdentity44(_pStack[0]);
		_lastError = GLT_STACK_NOERROR;
	}

	~GLMatrixStack(void) {
		delete [] this->_pStack;
	}

	inline void LoadIdentity(void) { 
		m3dLoadIdentity44(this->_pStack[_stackIndex]); 
	}

	inline void LoadMatrix(const M3DMatrix44f mMatrix) { 
		m3dCopyMatrix44(this->_pStack[_stackIndex], mMatrix); 
	}

	inline void LoadMatrix(GLFrame& frame) {
		M3DMatrix44f m;
		frame.GetMatrix(m);
		this->LoadMatrix(m);
	}

	inline void MultMatrix(const M3DMatrix44f mMatrix) {
		M3DMatrix44f mTemp;
		m3dCopyMatrix44(mTemp, _pStack[_stackIndex]);
		m3dMatrixMultiply44(_pStack[_stackIndex], mTemp, mMatrix);
	}

	inline void MultMatrix(GLFrame& frame) {
		M3DMatrix44f m;
		frame.GetMatrix(m);
		this->MultMatrix(m);
	}

	inline void PushMatrix(void) {
		if(_stackIndex < _stackDepth) {
			_stackIndex++;
			m3dCopyMatrix44(_pStack[_stackIndex], _pStack[_stackIndex-1]);
		}
		else
			_lastError = GLT_STACK_OVERFLOW;
	}

	inline void PopMatrix(void) {
		if(_stackIndex > 0)
			_stackIndex--;
		else
			_lastError = GLT_STACK_UNDERFLOW;
	}

	void Scale(GLfloat x, GLfloat y, GLfloat z) {
		M3DMatrix44f mTemp, mScale;
		m3dScaleMatrix44(mScale, x, y, z);
		m3dCopyMatrix44(mTemp, _pStack[_stackIndex]);
		m3dMatrixMultiply44(_pStack[_stackIndex], mTemp, mScale);
	}


	void Translate(GLfloat x, GLfloat y, GLfloat z) {
		M3DMatrix44f mTemp, mScale;
		m3dTranslationMatrix44(mScale, x, y, z);
		m3dCopyMatrix44(mTemp, _pStack[_stackIndex]);
		m3dMatrixMultiply44(_pStack[_stackIndex], mTemp, mScale);			
	}

	void Rotate(GLfloat angle, GLfloat x, GLfloat y, GLfloat z) {
		M3DMatrix44f mTemp, mRotate;
		m3dRotationMatrix44(mRotate, float(m3dDegToRad(angle)), x, y, z);
		m3dCopyMatrix44(mTemp, _pStack[_stackIndex]);
		m3dMatrixMultiply44(_pStack[_stackIndex], mTemp, mRotate);
	}


	// I've always wanted vector versions of these
	void Scalev(const M3DVector3f vScale) {
		M3DMatrix44f mTemp, mScale;
		m3dScaleMatrix44(mScale, vScale);
		m3dCopyMatrix44(mTemp, _pStack[_stackIndex]);
		m3dMatrixMultiply44(_pStack[_stackIndex], mTemp, mScale);
	}


	void Translatev(const M3DVector3f vTranslate) {
		M3DMatrix44f mTemp, mTranslate;
		m3dLoadIdentity44(mTranslate);
		m3dSetMatrixColumn44(mTranslate, vTranslate, 3);
		m3dCopyMatrix44(mTemp, _pStack[_stackIndex]);
		m3dMatrixMultiply44(_pStack[_stackIndex], mTemp, mTranslate);
	}


	void Rotatev(GLfloat angle, M3DVector3f vAxis) {
		M3DMatrix44f mTemp, mRotation;
		m3dRotationMatrix44(mRotation, float(m3dDegToRad(angle)), vAxis[0], vAxis[1], vAxis[2]);
		m3dCopyMatrix44(mTemp, _pStack[_stackIndex]);
		m3dMatrixMultiply44(_pStack[_stackIndex], mTemp, mRotation);
	}


	// I've also always wanted to be able to do this
	void PushMatrix(const M3DMatrix44f mMatrix) {
		if(_stackIndex < _stackDepth) {
			_stackIndex++;
			m3dCopyMatrix44(_pStack[_stackIndex], mMatrix);
		}
		else
			_lastError = GLT_STACK_OVERFLOW;
	}

	void PushMatrix(GLFrame& frame) {
		M3DMatrix44f m;
		frame.GetMatrix(m);
		this->PushMatrix(m);
	}

	// Two different ways to get the matrix
	const M3DMatrix44f& GetMatrix(void) { return _pStack[_stackIndex]; }
	void GetMatrix(M3DMatrix44f mMatrix) { m3dCopyMatrix44(mMatrix, _pStack[_stackIndex]); }


	inline GLT_STACK_ERROR GetLastError(void) {
		GLT_STACK_ERROR retval = _lastError;
		_lastError = GLT_STACK_NOERROR;
		return retval; 
	}

protected:
	GLT_STACK_ERROR		_lastError;
	int					_stackDepth;
	int					_stackIndex;
	M3DMatrix44f*		_pStack;
};

#endif
