#pragma once
#include "Arithmetic.h"
#include <Windows.h>

class CameraControl
{
private:
	Float3 pos;
	Float3 target;
	Float3 upVector;

	float fov;
	float zfar;
	float znear;
	float width;
	float height;

	POINT lFulcrum;
	POINT rFulcrum;
	POINT cFulcrum;
	Float3 attitude;
public:
	CameraControl();

	Float3* getPos()	{ return &pos; }
	Float3* getTarget()	{ return &target; }
	Float3* getUp()		{ return &upVector; }

	float* getFov()		{ return &fov; }
	float* getFar()		{ return &zfar; }
	float* getNear()	{ return &znear; }
	float* getWidth()	{ return &width; }
	float* getHeight()	{ return &height; }

	Float4x4 getView()const;
	Float4x4 getProjection()const;
	Float4x4 getOrthographic()const;

	/// <summary>
	/// �J�����̍X�V�����܂��B
	/// </summary>
	/// <param name="elapsedTime">�o�ߎ���</param>
	/// <param name="moveSpeed">�ړ����x</param>
	/// <param name="rotationSpeed">�p���x</param>
	void update(float elapsedTime,
		float moveSpeed = 32.0f,
		float rotationSpeed = (DirectX::XM_PI * 2.0f));

};