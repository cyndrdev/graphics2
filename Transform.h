#pragma once
#include "DirectXCore.h"

class Transform
{
public:
	Transform();
	~Transform();

	// mutators
	void Translate(XMFLOAT3 offset);
	void Translate(float x, float y, float z);

	void Rotate(XMFLOAT3 rotation);
	void Rotate(float roll, float pitch, float yaw);

	void Scale(XMFLOAT3 scale);
	void Scale(float x, float y, float z);
	void Scale(float scale);

	// setters
	void SetPosition(XMFLOAT3 position);
	void SetPosition(float x, float y, float z);

	void SetRotation(XMFLOAT3 rotation);
	void SetRotation(float roll, float pitch, float yaw);

	void SetScale(XMFLOAT3 scale);
	void SetScale(float x, float y, float z);
	void SetScale(float scale);

	// getters
	XMVECTOR GetPosition();
	XMVECTOR GetRotation();
	XMVECTOR GetScale();

	XMMATRIX GetWorldTransform(void);
private:
	XMFLOAT4 rotation_;
	XMFLOAT3 position_;
	XMFLOAT3 scale_;
};

