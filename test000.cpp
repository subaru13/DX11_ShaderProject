#include "include.h"
#include "painter/SpritePainter.h"
#include <vector>

/*
	変数宣言
*/
using Vertex = SpritePainter::Vertex;

struct Transform
{
	Float2 mSize{};
	float mRotation{};
	Float2 mPos{};

	Matrix getMatrix() const
	{
		return XMMatrixScaling(mSize.x,mSize.y, 0.0f) *
			XMMatrixRotationZ(mRotation) *
			XMMatrixTranslation(mPos.x, mPos.y, 0.0f);
	}
};

void initVertices(Vertex* vertices)
{
	vertices[0].mPos = { -0.5f,-0.5f };
	vertices[0].mUV = { 0.0f,0.0f };
	vertices[0].mColor = { 1.0f,1.0f,1.0f,1.0f };

	vertices[1].mPos = { 0.5f,-0.5f };
	vertices[1].mUV = { 1.0f,0.0f };
	vertices[1].mColor = { 1.0f,1.0f,1.0f,1.0f };

	vertices[2].mPos = { -0.5f,0.5f };
	vertices[2].mUV = { 0.0f,1.0f };
	vertices[2].mColor = { 1.0f,1.0f,1.0f,1.0f };

	vertices[3].mPos = { 0.5f,0.5f };
	vertices[3].mUV = { 1.0f,1.0f };
	vertices[3].mColor = { 1.0f,1.0f,1.0f,1.0f };
}

Matrix screenToClip(const Float2& resolution)
{
	const float w = resolution.x * 0.5f;
	const float h = resolution.y * 0.5f;
	return XMMatrixInverse(nullptr,
		XMMatrixSet(
		w,		0.0f,	0.0f,	0.0f,
		0.0f,	-h,		0.0f,	0.0f,
		0.0f,	0.0f,	1.0f,	0.0f,
		w,		h,		0.0f,	1.0f));
}

void transformVertices(Vertex* vertices, const Transform& transform, const Float2& resolution)
{
	Transform tempTransform = transform;
	tempTransform.mRotation = 0;
	auto transformMatrix = tempTransform.getMatrix();
	XMStoreFloat2(&vertices[0].mUV, XMVector3TransformCoord(XMLoadFloat2(&vertices[0].mPos), transformMatrix));
	XMStoreFloat2(&vertices[1].mUV, XMVector3TransformCoord(XMLoadFloat2(&vertices[1].mPos), transformMatrix));
	XMStoreFloat2(&vertices[2].mUV, XMVector3TransformCoord(XMLoadFloat2(&vertices[2].mPos), transformMatrix));
	XMStoreFloat2(&vertices[3].mUV, XMVector3TransformCoord(XMLoadFloat2(&vertices[3].mPos), transformMatrix));
	transformMatrix = transform.getMatrix() * screenToClip(resolution);
	XMStoreFloat2(&vertices[0].mPos, XMVector3TransformCoord(XMLoadFloat2(&vertices[0].mPos), transformMatrix));
	XMStoreFloat2(&vertices[1].mPos, XMVector3TransformCoord(XMLoadFloat2(&vertices[1].mPos), transformMatrix));
	XMStoreFloat2(&vertices[2].mPos, XMVector3TransformCoord(XMLoadFloat2(&vertices[2].mPos), transformMatrix));
	XMStoreFloat2(&vertices[3].mPos, XMVector3TransformCoord(XMLoadFloat2(&vertices[3].mPos), transformMatrix));
}

void normalizeOfUV(Vertex* vertices, const Float2& resolution)
{
	auto inverseResolution = XMVectorDivide(XMVectorReplicate(1.0f), XMLoadFloat2(&resolution));
	
	XMStoreFloat2(&vertices[0].mUV, XMVectorMultiply(XMLoadFloat2(&vertices[0].mUV), inverseResolution));
	XMStoreFloat2(&vertices[1].mUV, XMVectorMultiply(XMLoadFloat2(&vertices[1].mUV), inverseResolution));
	XMStoreFloat2(&vertices[2].mUV, XMVectorMultiply(XMLoadFloat2(&vertices[2].mUV), inverseResolution));
	XMStoreFloat2(&vertices[3].mUV, XMVectorMultiply(XMLoadFloat2(&vertices[3].mUV), inverseResolution));
}

void setVertices(Vertex* vertices, const Transform& transform, const Float2& resolution)
{
	initVertices(vertices);
	transformVertices(vertices, transform, resolution);
	normalizeOfUV(vertices, resolution);
}

Vertex vertices[4]{};
const Float2 resolution{ (float)SCREEN_WIDTH,(float)SCREEN_HEIGHT };
SpritePainter* spritePainter{ nullptr };
VertexBuffer vertexBuffer{};
ShaderResource shaderResource{};
Transform transform{};

/*
	初期化処理
	@ system = システムのアドレス
*/
void init(DX11System* dx11System)
{
	spritePainter = new SpritePainter(dx11System->d3d11Device.Get());
	loadShaderResource(dx11System->d3d11Device.Get(), &shaderResource, L"asset\\img10.jpg");
	transform.mSize.x = 256.0f;
	transform.mSize.y = 256.0f;
}

/*
	更新処理
	@ elapsedTime = 経過時間
*/
void update(float elapsedTime)
{
	if (Mouse::instance()->getL().hold())
	{
		transform.mRotation += elapsedTime * XM_PI;
	}
	transform.mPos.x = (float)Mouse::instance()->getPos().x;
	transform.mPos.y = (float)Mouse::instance()->getPos().y;
	setVertices(vertices, transform, resolution);
}

/*
	描画処理
	@ system = システムのアドレス
*/
void draw(DX11System* dx11System)
{
	createVertexBuffer(dx11System->d3d11Device.Get(), &vertexBuffer, sizeof(Vertex), 4, vertices);
	spritePainter->drawBegin(dx11System->d3d11DeviceContext.Get());
	spritePainter->draw(dx11System->d3d11DeviceContext.Get(), &shaderResource, &vertexBuffer, nullptr);
	spritePainter->drawEnd(dx11System->d3d11DeviceContext.Get());
}

/*
	終了処理
*/
void uninit()
{
	delete spritePainter;
}
