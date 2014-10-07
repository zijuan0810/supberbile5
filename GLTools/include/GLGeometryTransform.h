// GLGeometryTransform
#ifndef __GLT_GEOMETRY_PIPELINE
#define __GLT_GEOMETRY_PIPELINE

#include <GLTools.h>

class GLGeometryTransform
{
public:
	GLGeometryTransform(void) {}

	void SetModelViewMatrixStack(GLMatrixStack& mModelView) { 
		_mvMatrixStack = &mModelView; 
	}

	void SetProjectionMatrixStack(GLMatrixStack& mProjection) { 
		_pjMatrixStack = &mProjection; 
	}

	void SetMatrixStacks(GLMatrixStack& mModelView, GLMatrixStack& mProjection) {
		_mvMatrixStack = &mModelView;
		_pjMatrixStack = &mProjection;
	}

	const M3DMatrix44f& GetModelViewProjectionMatrix(void) {
		m3dMatrixMultiply44(_mvProjection, _pjMatrixStack->GetMatrix(), _mvMatrixStack->GetMatrix());
		return _mvProjection;
	}

	const M3DMatrix44f& GetModelViewMatrix(void) { 
		return _mvMatrixStack->GetMatrix(); 
	}

	const M3DMatrix44f& GetProjectionMatrix(void) { 
		return _pjMatrixStack->GetMatrix(); 
	}

	const M3DMatrix33f& GetNormalMatrix(bool bNormalize = false) {
		m3dExtractRotationMatrix33(_normalMatrix, GetModelViewMatrix());

		if(bNormalize) {
			m3dNormalizeVector3(&_normalMatrix[0]);
			m3dNormalizeVector3(&_normalMatrix[3]);
			m3dNormalizeVector3(&_normalMatrix[6]);
		}

		return _normalMatrix;
	}

protected:
	M3DMatrix44f	_mvProjection;
	M3DMatrix33f	_normalMatrix;

	GLMatrixStack*	_mvMatrixStack;
	GLMatrixStack*	_pjMatrixStack;
};

#endif
