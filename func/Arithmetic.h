#ifndef INCLUDE_ARITHMETIC
#define INCLUDE_ARITHMETIC

#include <math.h>
#include <DirectXMath.h>
#include <yvals_core.h>

/****************************************************************
	Square.
****************************************************************/
_NODISCARD inline float pow2(const float& x)
{
	return (x * x);
}

/****************************************************************
	Convert degrees to radians.
****************************************************************/
_NODISCARD inline float toRadian(const float& x)
{
	return (x * 0.0174533f);
}

/****************************************************************
	Convert radians to degrees.
****************************************************************/
_NODISCARD inline float toAngle(const float& x)
{
	return (x * 57.29577951308f);
}

/****************************************************************
	Normalize radians.
****************************************************************/
inline float normalizeRadian(float& radian)
{
	while (radian > DirectX::XM_PI) { radian -= (DirectX::XM_PI * 2.0f); }
	while (radian < -DirectX::XM_PI) { radian += (DirectX::XM_PI * 2.0f); }
	return radian;
}

/****************************************************************
	Floating point equality judgment.
****************************************************************/
_NODISCARD inline bool nearlyEqual(float a, float b, float e = FLT_EPSILON)
{
	return fabsf(a - b) <= e;
}

/****************************************************************
	It is an 8-byte structure.
	There are float type x, y elements.
	It inherits DirectX :: XMFLOAT2 and has some operators.
****************************************************************/
struct Float2 :public DirectX::XMFLOAT2
{
	Float2() :DirectX::XMFLOAT2(0.0f, 0.0f) {}
	Float2(float x, float y) :DirectX::XMFLOAT2(x, y) {}
	Float2(const Float2& v) :DirectX::XMFLOAT2(v.x, v.y) {}
	_NODISCARD Float2	operator+	()const { return { x, y }; }
	_NODISCARD Float2	operator-	()const { return { -x, -y }; }
	_NODISCARD Float2	operator+	(Float2 v)const { return { x + v.x,y + v.y }; }
	_NODISCARD Float2	operator-	(Float2 v)const { return { x - v.x,y - v.y }; }
	_NODISCARD Float2	operator*	(Float2 v)const { return { x * v.x,y * v.y }; }
	_NODISCARD Float2	operator*	(float s)const { return { x * s,y * s }; }
	_NODISCARD Float2	operator/	(Float2 v)const { return { x / v.x,y / v.y }; }
	_NODISCARD Float2	operator/	(float s)const { return { x / s,y / s }; }
	Float2	operator+=	(Float2 v) { return *this = (*this + v); }
	Float2	operator-=	(Float2 v) { return *this = (*this - v); }
	Float2	operator*=	(Float2 v) { return *this = (*this * v); }
	Float2	operator*=	(float s) { return *this = (*this * s); }
	Float2	operator/=	(Float2 v) { return *this = (*this / v); }
	Float2	operator/=	(float s) { return *this = (*this / s); }
	_NODISCARD bool	operator==	(Float2 v)const { return x == v.x && y == v.y; }
	_NODISCARD bool	operator!=	(Float2 v)const { return x != v.x || y != v.y; }
};

/****************************************************************
	Operator with a scalar whose vector is on the left.
****************************************************************/
_NODISCARD inline Float2 operator*(const float& s, const Float2& v) { return { s * v.x,s * v.y }; }
_NODISCARD inline Float2 operator/(const float& s, const Float2& v) { return { s / v.x,s / v.y }; }

/****************************************************************
	Normalize a 2D vector
****************************************************************/
_NODISCARD inline Float2 vec2Normalize(const Float2& v)
{
	float length = sqrtf((v.x * v.x) + (v.y * v.y));
	return Float2(v.x / length, v.y / length);
}

/****************************************************************
	Arithmetic of inner product of 2D vectors.
****************************************************************/
_NODISCARD inline float vec2Dot(const Float2& v1, const Float2& v2)
{
	return ((v1.x * v2.x) + (v1.y * v2.y));
}

/****************************************************************
	Arithmetic the cross product of 2D vectors.
****************************************************************/
_NODISCARD inline float vec2Cross(const Float2& v1, const Float2& v2)
{
	return v1.x * v2.y - v1.y * v2.x;
}

/****************************************************************
	Calculate the magnitude (length) of a 2D vector.
****************************************************************/
_NODISCARD inline float vec2Length(const Float2& v)
{
	return sqrtf((v.x * v.x) + (v.y * v.y));
}

/****************************************************************
	Calculates the square of
	the magnitude (length) of a 2D vector.
****************************************************************/
_NODISCARD inline float vec2LengthSq(const Float2& v)
{
	return ((v.x * v.x) + (v.y * v.y));
}

/****************************************************************
	Determine if the 2D vector is in a vertical relationship.
****************************************************************/
_NODISCARD inline bool vec2IsVertical(const Float2& v1, const Float2& v2)
{
	float d = vec2Dot(v1, v2);
	return (-FLT_EPSILON < d&& d < FLT_EPSILON);
}

/****************************************************************
	Determine if the 2D vectors are in parallel.
****************************************************************/
_NODISCARD inline bool vec2IsParallel(const Float2& v1, const Float2& v2)
{
	float d = vec2Cross(v1, v2);
	d *= d;
	return (-FLT_EPSILON < d&& d < FLT_EPSILON);
}

/****************************************************************
	Determine if the 2D vector has a sharp relationship.
****************************************************************/
_NODISCARD inline bool vec2IsSharp(const Float2& v1, const Float2& v2)
{
	return (vec2Dot(v1, v2) >= 0.0f);
}

/****************************************************************
	It is an 12-byte structure.
	There are float type x, y, z elements.
	It inherits DirectX :: XMFLOAT3 and has some operators.
****************************************************************/
struct Float3 :public DirectX::XMFLOAT3
{
	Float3() :DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f) {}
	Float3(float x, float y, float z) :DirectX::XMFLOAT3(x, y, z) {}
	Float3(const Float2& v, float z = 0.0f) :DirectX::XMFLOAT3(v.x, v.y, z) {}
	Float3(const Float3& v) :DirectX::XMFLOAT3(v.x, v.y, v.z) {}
	_NODISCARD Float3	operator+	()const { return { x, y,z }; }
	_NODISCARD Float3	operator-	()const { return { -x, -y,-z }; }
	_NODISCARD Float3	operator+	(Float3 v)const { return { x + v.x,y + v.y,z + v.z }; }
	_NODISCARD Float3	operator-	(Float3 v)const { return { x - v.x,y - v.y,z - v.z }; }
	_NODISCARD Float3	operator*	(Float3 v)const { return { x * v.x,y * v.y,z * v.z }; }
	_NODISCARD Float3	operator*	(float s)const { return { x * s,y * s,z * s }; }
	_NODISCARD Float3	operator/	(Float3 v)const { return { x / v.x,y / v.y,z / v.z }; }
	_NODISCARD Float3	operator/	(float s)const { return { x / s,y / s,z / s }; }
	Float3	operator+=	(Float3 v) { return *this = (*this + v); }
	Float3	operator-=	(Float3 v) { return *this = (*this - v); }
	Float3	operator*=	(Float3 v) { return *this = (*this * v); }
	Float3	operator*=	(float s) { return *this = (*this * s); }
	Float3	operator/=	(Float3 v) { return *this = (*this / v); }
	Float3	operator/=	(float s) { return *this = (*this / s); }
	_NODISCARD bool	operator==	(Float3 v)const { return x == v.x && y == v.y && z == v.z; }
	_NODISCARD bool	operator!=	(Float3 v)const { return x != v.x || y != v.y || z != v.z; }
};

/****************************************************************
	Operator with a scalar whose vector is on the left.
****************************************************************/
_NODISCARD inline Float3 operator*(const float& s, const Float3& v) { return { s * v.x,s * v.y,s * v.z }; }
_NODISCARD inline Float3 operator/(const float& s, const Float3& v) { return { s / v.x,s / v.y,s / v.z }; }

/****************************************************************
	Normalize a 3D vector
****************************************************************/
_NODISCARD inline Float3 vec3Normalize(const Float3& v)
{
	float length = sqrtf((v.x * v.x) + (v.y * v.y) + (v.z * v.z));
	return Float3(v.x / length, v.y / length, v.z / length);
}

/****************************************************************
	Arithmetic of inner product of 3D vectors.
****************************************************************/
_NODISCARD inline float vec3Dot(const Float3& v1, const Float3& v2)
{
	return ((v1.x * v2.x) + (v1.y * v2.y) + (v1.z * v2.z));
}

/****************************************************************
	Arithmetic the cross product of 3D vectors.
****************************************************************/
_NODISCARD inline Float3 vec3Cross(const Float3& v1, const Float3& v2)
{
	float x = (v1.y * v2.z) - (v1.z * v2.y);
	float y = (v1.z * v2.x) - (v1.x * v2.z);
	float z = (v1.x * v2.y) - (v1.y * v2.x);
	return Float3(x, y, z);
}

/****************************************************************
	Calculate the magnitude (length) of a 3D vector.
****************************************************************/
_NODISCARD inline float vec3Length(const Float3& v)
{
	return sqrtf((v.x * v.x) + (v.y * v.y) + (v.z * v.z));
}

/****************************************************************
	Calculates the square of
	the magnitude (length) of a 3D vector.
****************************************************************/
_NODISCARD inline float vec3LengthSq(const Float3& v)
{
	return ((v.x * v.x) + (v.y * v.y) + (v.z * v.z));
}

/****************************************************************
	Determine if the 3D vector is in a vertical relationship.
****************************************************************/
_NODISCARD inline bool vec3IsVertical(const Float3& v1, const Float3& v2)
{
	float d = vec3Dot(v1, v2);
	return (-FLT_EPSILON < d&& d < FLT_EPSILON);
}

/****************************************************************
	Determine if the 3D vectors are in parallel.
****************************************************************/
_NODISCARD inline bool vec3IsParallel(const Float3& v1, const Float3& v2)
{
	float d = vec3LengthSq(vec3Cross(v1, v2));
	return (-FLT_EPSILON < d&& d < FLT_EPSILON);
}

/****************************************************************
	Determine if the 3D vector has a sharp relationship.
****************************************************************/
_NODISCARD inline bool vec3IsSharp(const Float3& v1, const Float3& v2)
{
	return (vec3Dot(v1, v2) >= 0.0f);
}

typedef DirectX::XMFLOAT4 Float4;

/****************************************************************
	Convert from color code to RGBA.
****************************************************************/
_NODISCARD inline Float4 colorCodeToRGBA(unsigned long colorCode)
{
	Float4 rgba{};

	rgba.x = ((colorCode >> 24) & 0x000000FF) / 255.0f;
	rgba.y = ((colorCode >> 16) & 0x000000FF) / 255.0f;
	rgba.z = ((colorCode >> 8) & 0x000000FF) / 255.0f;
	rgba.w = ((colorCode >> 0) & 0x000000FF) / 255.0f;

	return rgba;
}

typedef DirectX::XMFLOAT4X4	Float4x4;
typedef DirectX::XMMATRIX Matrix;

#define RotationMatrix(angles)	XMMatrixRotationRollPitchYaw(angles.x,angles.y,angles.z)
#define TransformMatrix(pos)	XMMatrixTranslation(pos.x, pos.y, pos.z)
#define ScalingMatrix(scales)	XMMatrixScaling(scales.x, scales.y, scales.z)

#define _SCALAR_TO_FLOAT2(scalar) (FLOAT2(scalar,scalar))
#define _SCALAR_TO_FLOAT3(scalar) (Float3(scalar,scalar,scalar))
#define _SCALAR_TO_FLOAT4(scalar) (FLOAT4(scalar,scalar,scalar,scalar))

static inline Float4x4 matrixToFloat4x4(const Matrix& matrix)
{
	Float4x4 float4x4;
	DirectX::XMStoreFloat4x4(&float4x4, matrix);
	return float4x4;
}

using namespace DirectX;

#endif
