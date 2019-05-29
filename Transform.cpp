#include "Transform.h"

Transform::Transform() :
	position_(0.0f, 0.0f, 0.0f),
	rotation_(0.0f, 0.0f, 0.0f, 0.0f),
	scale_(1.0f, 1.0f, 1.0f)
{
	XMStoreFloat4(
		&rotation_,
		XMQuaternionRotationRollPitchYaw(0.0f, 0.0f, 0.0f)
	);
}

Transform::~Transform()
{
}

void Transform::Translate(XMFLOAT3 offset)
{
	XMStoreFloat3(
		&position_,
		XMLoadFloat3(&offset) + XMLoadFloat3(&position_)
	);
}

void Transform::Translate(float x, float y, float z)
{
	Translate(XMFLOAT3(x, y, z));
}

void Transform::Rotate(XMFLOAT3 rotation)
{
	XMStoreFloat4(
		&rotation_,
		XMQuaternionMultiply(
			XMLoadFloat4(&rotation_),
			XMQuaternionRotationRollPitchYawFromVector(
				XMLoadFloat3(&rotation)
			)
		)
	);
}

void Transform::Rotate(float roll, float pitch, float yaw)
{
	Rotate(XMFLOAT3(roll, pitch, yaw));
}

void Transform::Scale(XMFLOAT3 scale)
{
	XMStoreFloat3(
		&scale_,
		XMLoadFloat3(&scale) * XMLoadFloat3(&scale_)
	);
}

void Transform::Scale(float x, float y, float z)
{
	Scale(XMFLOAT3(x, y, z));
}

void Transform::Scale(float scale)
{
	Scale(XMFLOAT3(scale, scale, scale));
}

void Transform::SetPosition(XMFLOAT3 position)
{
	XMStoreFloat3(&position_, XMLoadFloat3(&position));
}

void Transform::SetPosition(float x, float y, float z)
{
	SetPosition(XMFLOAT3(x, y, z));
}

void Transform::SetRotation(XMFLOAT3 rotation)
{
	XMStoreFloat4(
		&rotation_,
		XMQuaternionRotationRollPitchYawFromVector(
			XMLoadFloat3(&rotation)
		)
	);
}

void Transform::SetRotation(float roll, float pitch, float yaw)
{
	SetRotation(XMFLOAT3(roll, pitch, yaw));
}

void Transform::SetScale(XMFLOAT3 scale)
{
	XMStoreFloat3(&scale_, XMLoadFloat3(&scale));
}

void Transform::SetScale(float x, float y, float z)
{
	SetScale(XMFLOAT3(x, y, z));
}

void Transform::SetScale(float scale)
{
	SetScale(XMFLOAT3(scale, scale, scale));
}

XMVECTOR Transform::GetPosition()
{
	return XMLoadFloat3(&position_);
}

XMVECTOR Transform::GetRotation()
{
	return XMLoadFloat4(&rotation_);
}

XMVECTOR Transform::GetScale()
{
	return XMLoadFloat3(&scale_);
}

XMMATRIX Transform::GetWorldTransform(void)
{
	return XMMatrixScalingFromVector(XMLoadFloat3(&scale_))
		* XMMatrixRotationQuaternion(XMLoadFloat4(&rotation_))
		* XMMatrixTranslationFromVector(XMLoadFloat3(&position_));
}
