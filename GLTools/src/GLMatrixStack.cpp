#include "GLMatrixStack.h"

GLMatrixStack::GLMatrixStack(int iStackDepth /* = 64 */)
	: _stackDepth(iStackDepth)
	, _stackIndex(0)
	, _lastError(GLT_STACK_NOERROR)
{
	_stackes = new M3DMatrix44f[iStackDepth];
	m3dLoadIdentity44(_stackes[0]);
}

GLMatrixStack::~GLMatrixStack(void) 
{
	delete[] this->_stackes;
}

GLMatrixStack& GLMatrixStack::operator*=(const M3DMatrix44f matrix)
{
	M3DMatrix44f mTemp;
	m3dCopyMatrix44(mTemp, _stackes[_stackIndex]);
	m3dMatrixMultiply44(_stackes[_stackIndex], mTemp, matrix);

	return *this;
}

GLMatrixStack& GLMatrixStack::operator*=(GLFrame& frame)
{
	M3DMatrix44f matrix;
	frame.GetMatrix(matrix);

	M3DMatrix44f mTemp;
	m3dCopyMatrix44(mTemp, _stackes[_stackIndex]);
	m3dMatrixMultiply44(_stackes[_stackIndex], mTemp, matrix);

	return *this;
}
