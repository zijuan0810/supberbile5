// Frame.h
// Implementation of the GLFrame Class

#include <math3d.h>

#ifndef _ORTHO_FRAME_
#define _ORTHO_FRAME_

/**
* The GLFrame (OrthonormalFrame) class. Possibly the most useful little piece of 3D graphics
* code for OpenGL immersive environments.
*/
class GLFrame
{
public:
	/**
	* Default position and orientation. At the origin, looking
	* down the positive Z axis (right handed coordinate system).
	*/
	GLFrame(void) {
		// At origin
		_vectorOrigin[0] = 0.0f; _vectorOrigin[1] = 0.0f; _vectorOrigin[2] = 0.0f; 

		// Up is up (+Y)
		_vectorUp[0] = 0.0f; _vectorUp[1] = 1.0f; _vectorUp[2] = 0.0f;

		// Forward is -Z (default OpenGL)
		_vectorForward[0] = 0.0f; _vectorForward[1] = 0.0f; _vectorForward[2] = -1.0f;
	}


	/**
	 * Set Location
	 */
	inline void SetOrigin(const M3DVector3f vPoint) {
		m3dCopyVector3(_vectorOrigin, vPoint); 
	}

	inline void SetOrigin(float x, float y, float z) { 
		_vectorOrigin[0] = x; _vectorOrigin[1] = y; _vectorOrigin[2] = z; 
	}

	inline void GetOrigin(M3DVector3f vPoint) {
		m3dCopyVector3(vPoint, _vectorOrigin); 
	}

	inline float GetOriginX(void) { return _vectorOrigin[0]; }
	inline float GetOriginY(void) { return _vectorOrigin[1]; } 
	inline float GetOriginZ(void) { return _vectorOrigin[2]; }

	/**
	 * Set Forward Direction
	 */
	inline void SetForwardVector(const M3DVector3f vDirection) {
		m3dCopyVector3(_vectorForward, vDirection); 
	}

	inline void SetForwardVector(float x, float y, float z) { 
		_vectorForward[0] = x; 
		_vectorForward[1] = y; 
		_vectorForward[2] = z; 
	}

	inline void GetForwardVector(M3DVector3f vVector) { 
		m3dCopyVector3(vVector, _vectorForward); 
	}


	/**
	 * Set Up Direction
	 */
	inline void SetUpVector(const M3DVector3f vDirection) {
		m3dCopyVector3(_vectorUp, vDirection); 
	}

	inline void SetUpVector(float x, float y, float z) { 
		_vectorUp[0] = x; 
		_vectorUp[1] = y; 
		_vectorUp[2] = z; 
	}

	inline void GetUpVector(M3DVector3f vVector) { 
		m3dCopyVector3(vVector, _vectorUp); 
	}


	/**
	 * Get Axes
	 */
	inline void GetZAxis(M3DVector3f vVector) { GetForwardVector(vVector); }
	inline void GetYAxis(M3DVector3f vVector) { GetUpVector(vVector); }
	inline void GetXAxis(M3DVector3f vVector) { 
		m3dCrossProduct3(vVector, _vectorUp, _vectorForward); 
	}


	/**
	 * moveTo along orthonormal axis... world or local
	 */
	inline void TranslateWorld(float x, float y, float z) { 
		_vectorOrigin[0] += x; 
		_vectorOrigin[1] += y; 
		_vectorOrigin[2] += z; 
	}

	inline void TranslateLocal(float x, float y, float z) { 
		this->MoveForward(z); 
		this->MoveUp(y); 
		this->MoveRight(x);	
	}


	// Move Forward (along Z axis)
	inline void MoveForward(float fDelta) {
		// Move along direction of front direction
		_vectorOrigin[0] += _vectorForward[0] * fDelta;
		_vectorOrigin[1] += _vectorForward[1] * fDelta;
		_vectorOrigin[2] += _vectorForward[2] * fDelta;
	}

	// Move along Y axis
	inline void MoveUp(float fDelta) {
		// Move along direction of up direction
		_vectorOrigin[0] += _vectorUp[0] * fDelta;
		_vectorOrigin[1] += _vectorUp[1] * fDelta;
		_vectorOrigin[2] += _vectorUp[2] * fDelta;
	}

	// Move along X axis
	inline void MoveRight(float fDelta) {
		// Move along direction of right vector
		M3DVector3f vCross;
		m3dCrossProduct3(vCross, _vectorUp, _vectorForward);

		_vectorOrigin[0] += vCross[0] * fDelta;
		_vectorOrigin[1] += vCross[1] * fDelta;
		_vectorOrigin[2] += vCross[2] * fDelta;
	}


	/**
	* Just assemble the matrix
	*/
	void GetMatrix(M3DMatrix44f outerMatrix, bool bRotationOnly = false) {
		// Calculate the right side (x) vector, drop it right into the matrix
		M3DVector3f vXAxis;
		m3dCrossProduct3(vXAxis, _vectorUp, _vectorForward);

		// Set matrix column does not fill in the fourth value...
		m3dSetMatrixColumn44(outerMatrix, vXAxis, 0);
		outerMatrix[3] = 0.0f;

		// Y Column
		m3dSetMatrixColumn44(outerMatrix, _vectorUp, 1);
		outerMatrix[7] = 0.0f;       

		// Z Column
		m3dSetMatrixColumn44(outerMatrix, _vectorForward, 2);
		outerMatrix[11] = 0.0f;

		// Translation (already done)
		if(bRotationOnly == true) {
			outerMatrix[12] = 0.0f;
			outerMatrix[13] = 0.0f;
			outerMatrix[14] = 0.0f;
		}
		else {
			m3dSetMatrixColumn44(outerMatrix, _vectorOrigin, 3);
		}

		outerMatrix[15] = 1.0f;
	}


	/**
	* Assemble( ’ºØ) the camera matrix
	*/
	void GetCameraMatrix(M3DMatrix44f outerMatrix, bool bRotationOnly = false) {
		M3DVector3f x, z;

		// Make rotation matrix
		// Z vector is reversed
		z[0] = -_vectorForward[0];
		z[1] = -_vectorForward[1];
		z[2] = -_vectorForward[2];

		// X vector = Y cross Z 
		m3dCrossProduct3(x, _vectorUp, z);

		// Matrix has no translation information and is transposed.... (rows instead of columns)
#define M(row,col)  outerMatrix[col*4+row]
		M(0, 0) = x[0];
		M(0, 1) = x[1];
		M(0, 2) = x[2];
		M(0, 3) = 0.0;
		M(1, 0) = _vectorUp[0];
		M(1, 1) = _vectorUp[1];
		M(1, 2) = _vectorUp[2];
		M(1, 3) = 0.0;
		M(2, 0) = z[0];
		M(2, 1) = z[1];
		M(2, 2) = z[2];
		M(2, 3) = 0.0;
		M(3, 0) = 0.0;
		M(3, 1) = 0.0;
		M(3, 2) = 0.0;
		M(3, 3) = 1.0;
#undef M

		if(bRotationOnly) {
			return;
		}

		// Apply translation too
		M3DMatrix44f trans, tempMatrix;
		m3dTranslationMatrix44(trans, -_vectorOrigin[0], -_vectorOrigin[1], -_vectorOrigin[2]);  

		m3dMatrixMultiply44(tempMatrix, outerMatrix, trans);

		// Copy result back into m
		memcpy(outerMatrix, tempMatrix, sizeof(float)*16);
	}


	/**
	* rotateTo around local Y
	*/
	void RotateLocalY(float fAngle) {
		M3DMatrix44f rotMat;

		// Just rotateTo around the up vector
		// Create a rotation matrix around my Up (Y) vector
		m3dRotationMatrix44(rotMat, fAngle, _vectorUp[0], _vectorUp[1], _vectorUp[2]);

		// rotateTo forward pointing vector (inlined 3x3 transform)
		M3DVector3f newVect;
		newVect[0] = rotMat[0] * _vectorForward[0] + rotMat[4] * _vectorForward[1] + rotMat[8] *  _vectorForward[2];	
		newVect[1] = rotMat[1] * _vectorForward[0] + rotMat[5] * _vectorForward[1] + rotMat[9] *  _vectorForward[2];	
		newVect[2] = rotMat[2] * _vectorForward[0] + rotMat[6] * _vectorForward[1] + rotMat[10] * _vectorForward[2];	
		m3dCopyVector3(_vectorForward, newVect);
	}


	/**
	* rotateTo around local Z
	*/
	void RotateLocalZ(float fAngle) {
		M3DMatrix44f rotMat;

		// Only the up vector needs to be rotated
		m3dRotationMatrix44(rotMat, fAngle, _vectorForward[0], _vectorForward[1], _vectorForward[2]);

		M3DVector3f newVect;
		newVect[0] = rotMat[0] * _vectorUp[0] + rotMat[4] * _vectorUp[1] + rotMat[8] *  _vectorUp[2];	
		newVect[1] = rotMat[1] * _vectorUp[0] + rotMat[5] * _vectorUp[1] + rotMat[9] *  _vectorUp[2];	
		newVect[2] = rotMat[2] * _vectorUp[0] + rotMat[6] * _vectorUp[1] + rotMat[10] * _vectorUp[2];	
		m3dCopyVector3(_vectorUp, newVect);
	}


	/**
	* rotateTo around local X
	*/
	void RotateLocalX(float fAngle) {
		M3DMatrix33f rotMat;
		M3DVector3f  localX;
		M3DVector3f  rotVec;

		// Get the local X axis
		m3dCrossProduct3(localX, _vectorUp, _vectorForward);

		// Make a Rotation Matrix
		m3dRotationMatrix33(rotMat, fAngle, localX[0], localX[1], localX[2]);

		// rotateTo Y, and Z
		m3dRotateVector(rotVec, _vectorUp, rotMat);
		m3dCopyVector3(_vectorUp, rotVec);

		m3dRotateVector(rotVec, _vectorForward, rotMat);
		m3dCopyVector3(_vectorForward, rotVec);
	}


	/**
	* Reset axes to make sure they are orthonormal. This should be called on occasion
	* if the matrix is long-lived and frequently transformed.
	*/
	void Normalize(void) {
		M3DVector3f vCross;

		// Calculate cross product of up and forward vectors
		m3dCrossProduct3(vCross, _vectorUp, _vectorForward);

		// Use result to recalculate forward vector
		m3dCrossProduct3(_vectorForward, vCross, _vectorUp);	

		// Also check for unit length...
		m3dNormalizeVector3(_vectorUp);
		m3dNormalizeVector3(_vectorForward);
	}


	/**
	* rotateTo in world coordinates...
	*/
	void RotateWorld(float fAngle, float x, float y, float z) {
		M3DMatrix44f rotMat;

		// Create the Rotation matrix
		m3dRotationMatrix44(rotMat, fAngle, x, y, z);

		M3DVector3f newVect;

		// Transform the up axis (inlined 3x3 rotation)
		newVect[0] = rotMat[0] * _vectorUp[0] + rotMat[4] * _vectorUp[1] + rotMat[8] *  _vectorUp[2];	
		newVect[1] = rotMat[1] * _vectorUp[0] + rotMat[5] * _vectorUp[1] + rotMat[9] *  _vectorUp[2];	
		newVect[2] = rotMat[2] * _vectorUp[0] + rotMat[6] * _vectorUp[1] + rotMat[10] * _vectorUp[2];	
		m3dCopyVector3(_vectorUp, newVect);

		// Transform the forward axis
		newVect[0] = rotMat[0] * _vectorForward[0] + rotMat[4] * _vectorForward[1] + rotMat[8] *  _vectorForward[2];	
		newVect[1] = rotMat[1] * _vectorForward[0] + rotMat[5] * _vectorForward[1] + rotMat[9] *  _vectorForward[2];	
		newVect[2] = rotMat[2] * _vectorForward[0] + rotMat[6] * _vectorForward[1] + rotMat[10] * _vectorForward[2];	
		m3dCopyVector3(_vectorForward, newVect);
	}


	/**
	* rotateTo around a local axis
	*/
	void RotateLocal(float fAngle, float x, float y, float z) {
		M3DVector3f vWorldVect;
		M3DVector3f vLocalVect;
		m3dLoadVector3(vLocalVect, x, y, z);

		LocalToWorld(vLocalVect, vWorldVect, true);
		RotateWorld(fAngle, vWorldVect[0], vWorldVect[1], vWorldVect[2]);
	}


	/**
	* Convert Coordinate Systems
	* This is pretty much, do the transformation represented by the rotation and position on the 
	* point. Is it better to stick to the convention that the destination always comes first, 
	* or use the conventions that "sounds" like the function...
	*/
	void LocalToWorld(const M3DVector3f vLocal, M3DVector3f vWorld, bool bRotOnly = false) {
		// Create the rotation matrix based on the vectors
		M3DMatrix44f rotMat;

		GetMatrix(rotMat, true);

		// Do the rotation (inline it, and remove 4th column...)
		vWorld[0] = rotMat[0] * vLocal[0] + rotMat[4] * vLocal[1] + rotMat[8] *  vLocal[2];	
		vWorld[1] = rotMat[1] * vLocal[0] + rotMat[5] * vLocal[1] + rotMat[9] *  vLocal[2];	
		vWorld[2] = rotMat[2] * vLocal[0] + rotMat[6] * vLocal[1] + rotMat[10] * vLocal[2];	

		// moveTo the point
		if(!bRotOnly) {
			vWorld[0] += _vectorOrigin[0];
			vWorld[1] += _vectorOrigin[1];
			vWorld[2] += _vectorOrigin[2];
		}
	}

	/**
	* Change world coordinates into "local" coordinates
	*/
	void WorldToLocal(const M3DVector3f vWorld, M3DVector3f vLocal) {
		// moveTo the origin
		M3DVector3f vNewWorld;
		vNewWorld[0] = vWorld[0] - _vectorOrigin[0];
		vNewWorld[1] = vWorld[1] - _vectorOrigin[1];
		vNewWorld[2] = vWorld[2] - _vectorOrigin[2];

		// Create the rotation matrix based on the vectors
		M3DMatrix44f rotMat;
		M3DMatrix44f invMat;
		GetMatrix(rotMat, true);

		// Do the rotation based on inverted matrix
		m3dInvertMatrix44(invMat, rotMat);

		vLocal[0] = invMat[0] * vNewWorld[0] + invMat[4] * vNewWorld[1] + invMat[8] *  vNewWorld[2];	
		vLocal[1] = invMat[1] * vNewWorld[0] + invMat[5] * vNewWorld[1] + invMat[9] *  vNewWorld[2];	
		vLocal[2] = invMat[2] * vNewWorld[0] + invMat[6] * vNewWorld[1] + invMat[10] * vNewWorld[2];	
	}


	/**
	* Transform a point by frame matrix
	*/
	void TransformPoint(M3DVector3f vPointSrc, M3DVector3f vPointDst) {
		M3DMatrix44f m;
		GetMatrix(m, false);    // rotateTo and translate
		vPointDst[0] = m[0] * vPointSrc[0] + m[4] * vPointSrc[1] + m[8] *  vPointSrc[2] + m[12];// * v[3];	 
		vPointDst[1] = m[1] * vPointSrc[0] + m[5] * vPointSrc[1] + m[9] *  vPointSrc[2] + m[13];// * v[3];	
		vPointDst[2] = m[2] * vPointSrc[0] + m[6] * vPointSrc[1] + m[10] * vPointSrc[2] + m[14];// * v[3];	
	}

	/**
	* rotateTo a vector by frame matrix
	*/
	void RotateVector(M3DVector3f vVectorSrc, M3DVector3f vVectorDst) {
		M3DMatrix44f m;
		GetMatrix(m, true);    // rotateTo only

		vVectorDst[0] = m[0] * vVectorSrc[0] + m[4] * vVectorSrc[1] + m[8] *  vVectorSrc[2];	 
		vVectorDst[1] = m[1] * vVectorSrc[0] + m[5] * vVectorSrc[1] + m[9] *  vVectorSrc[2];	
		vVectorDst[2] = m[2] * vVectorSrc[0] + m[6] * vVectorSrc[1] + m[10] * vVectorSrc[2];	
	}

protected:
	M3DVector3f _vectorOrigin;	// Where am I?
	M3DVector3f _vectorForward;	// Where am I going?
	M3DVector3f _vectorUp;		// Which way is up?
};


#endif
