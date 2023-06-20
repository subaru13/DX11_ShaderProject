#include "CameraControl.h"
#include "KeyInput.h"
#include "FrameworkConfig.h"

#define SWING_WIDTH		toRadian(60.0f)

CameraControl::CameraControl()
	:pos(0.0f, 0.0f, -10.0f),
	target(0.0f, 0.0f, 0.0f),
	upVector(0.0f, 1.0f, 0.0f),
	fov(toRadian(30.0f)),
	width(static_cast<float>(SCREEN_WIDTH)),
	height(static_cast<float>(SCREEN_HEIGHT)),
	znear(0.1f), zfar(1000.0f),
	lFulcrum(), rFulcrum(), cFulcrum()
{}

Float4x4 CameraControl::getView() const
{
	XMVECTOR eye{ XMLoadFloat3(&pos) };
	XMVECTOR focus;
	if (pos != target)
	{
		focus = XMLoadFloat3(&target);
	}
	else
	{
		focus = XMVectorSet(pos.x, pos.y, pos.z + 1.0f, 1.0f);
	}
	XMVECTOR up{ XMLoadFloat3(&upVector) };
	return matrixToFloat4x4(XMMatrixLookAtLH(eye, focus, up));
}

Float4x4 CameraControl::getProjection() const
{
	return matrixToFloat4x4(XMMatrixPerspectiveFovLH(fov, width / height, znear, zfar));
}

Float4x4 CameraControl::getOrthographic() const
{
	return matrixToFloat4x4(XMMatrixOrthographicLH(width, height, znear, zfar));
}

void CameraControl::update(float elapsedTime, float moveSpeed, float rotationSpeed)
{
	const Mouse* mouse = Mouse::instance();
	const Key& lKey = mouse->getL();
	const Key& rKey = mouse->getR();
	const Key& cKey = mouse->getC();
	const POINT& mousePos = mouse->getPos();
	if (lKey.down())
	{
		lFulcrum = mousePos;
	}
	else if (lKey.hold())
	{
		POINT drg = POINT{ mousePos.x - lFulcrum.x, mousePos.y - lFulcrum.y };

		if (fabsl(drg.x) > 64L)
		{
			if (vec3Dot(Float3(0, 1, 0), upVector) < 0)drg.x *= -1;
			attitude.y += drg.x > 0 ? elapsedTime * 0.174533f * rotationSpeed : -elapsedTime * 0.174533f * rotationSpeed;
		}

		if (fabsl(drg.y) > 64L)
		{
			attitude.x += drg.y > 0 ? elapsedTime * 0.174533f * rotationSpeed : -elapsedTime * 0.174533f * rotationSpeed;
		}

		if (attitude.x < -SWING_WIDTH)attitude.x = -SWING_WIDTH;
		if (attitude.x > SWING_WIDTH)attitude.x = SWING_WIDTH;

		Float4x4 rotation4x4{};
		XMStoreFloat4x4(&rotation4x4, RotationMatrix(attitude));
		target = pos + vec3Normalize(Float3(rotation4x4._31, rotation4x4._32, rotation4x4._33));
		upVector = vec3Normalize(Float3(rotation4x4._21, rotation4x4._22, rotation4x4._23));
	}

	if (rKey.down())
	{
		rFulcrum = mousePos;
	}
	else if (rKey.hold())
	{
		POINT drg = POINT{ mousePos.x - rFulcrum.x, mousePos.y - rFulcrum.y };
		Float4x4 rotation4x4{};
		XMStoreFloat4x4(&rotation4x4, RotationMatrix(attitude));

		Float3 forward = vec3Normalize(Float3(rotation4x4._31, rotation4x4._32, rotation4x4._33));
		Float3 right = vec3Normalize(Float3(rotation4x4._11, rotation4x4._12, rotation4x4._13));

		if (fabsl(drg.x) > 64L)
		{
			pos += drg.x > 0 ? elapsedTime * right * moveSpeed : -elapsedTime * right * moveSpeed;
		}

		if (fabsl(drg.y) > 64L)
		{
			pos += drg.y > 0 ? -elapsedTime * forward * moveSpeed : elapsedTime * forward * moveSpeed;
		}

		target = pos + forward;
	}


	if (cKey.down())
	{
		cFulcrum = mousePos;
	}
	else if (cKey.hold())
	{
		POINT drg = POINT{ mousePos.x - cFulcrum.x, mousePos.y - cFulcrum.y };
		Float4x4 rotation4x4{};
		XMStoreFloat4x4(&rotation4x4, RotationMatrix(attitude));

		Float3 forward = vec3Normalize(Float3(rotation4x4._31, rotation4x4._32, rotation4x4._33));
		Float3 right = vec3Normalize(Float3(rotation4x4._11, rotation4x4._12, rotation4x4._13));

		if (fabsl(drg.x) > 64L)
		{
			pos += drg.x > 0 ? elapsedTime * right * moveSpeed : -elapsedTime * right * moveSpeed;
		}

		if (fabsl(drg.y) > 64L)
		{
			pos += drg.y > 0 ? -elapsedTime * upVector * moveSpeed : elapsedTime * upVector * moveSpeed;
		}

		target = pos + forward;
	}
}
