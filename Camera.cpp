#include "Camera.h"

XMVECTOR defaultForward = XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);
XMVECTOR defaultRight = XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f);
XMVECTOR defaultUp = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

Camera::Camera()
{
    cameraPosition_ = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
    moveLeftRight_ = 0.0f;
    moveForwardBack_ = 0.0f;
    cameraYaw_ = 0.0f;
    cameraPitch_ = 0.0f;
    cameraRoll_ = 0.0f;
}

Camera::~Camera()
{
}

void Camera::SetPitch(float pitch)
{
    cameraPitch_ += XMConvertToRadians(pitch);
}

void Camera::SetTotalPitch(float pitch)
{
	cameraPitch_ = XMConvertToRadians(pitch);
}

float Camera::GetPitch() const
{
	return XMConvertToDegrees(cameraPitch_);
}

void Camera::SetYaw(float yaw)
{
    cameraYaw_ += XMConvertToRadians(yaw);
}

void Camera::SetTotalYaw(float yaw)
{
	cameraYaw_ = XMConvertToRadians(yaw);
}

float Camera::GetYaw() const
{
	return XMConvertToDegrees(cameraYaw_);
}

void Camera::SetRoll(float roll)
{
    cameraRoll_ += XMConvertToRadians(roll);
}

void Camera::SetTotalRoll(float roll)
{
	cameraRoll_ = XMConvertToRadians(roll);
}

float Camera::GetRoll() const
{
	return XMConvertToDegrees(cameraRoll_);
}

void Camera::SetLeftRight(float leftRight)
{
    moveLeftRight_ = leftRight;
}

void Camera::SetForwardBack(float forwardBack)
{
    moveForwardBack_ = forwardBack;
}

void Camera::SetRelativeY(float relY)
{
	cameraPosition_ = XMFLOAT4(
		cameraPosition_.x,
		cameraPosition_.y + relY,
		cameraPosition_.z,
		cameraPosition_.w
	);
}

bool Camera::IsFollowingNode()
{
	return (player_ != nullptr);
}

XMMATRIX Camera::GetViewMatrix(void)
{
    return XMLoadFloat4x4(&viewMatrix_);
}

XMVECTOR Camera::GetCameraPosition(void)
{
    return XMLoadFloat4(&cameraPosition_);
}

void Camera::FollowNode(SceneNodePointer node, float offset, float y_mod)
{
	player_ = node;
	if (player_ == nullptr) {
		yMod_ = 0.0f;
		offset_ = XMFLOAT3(0, 0, 0);
	}
	else {
		offset_ = XMFLOAT3(-offset, 0.0f, 0.0f);
		yMod_ = y_mod;
	}
}

void Camera::SetCameraPosition(float x, float y, float z)
{
    cameraPosition_ = XMFLOAT4(x, y, z, 0.0f);
}

void Camera::Update(void)
{

	while (cameraYaw_	>= XM_2PI)	cameraYaw_		-= XM_2PI;
	while (cameraPitch_ >= XM_2PI)	cameraPitch_	-= XM_2PI;
	while (cameraRoll_	>= XM_2PI)	cameraRoll_		-= XM_2PI;

	while (cameraYaw_ < 0.0f)		cameraYaw_		+= XM_2PI;
	while (cameraPitch_ < 0.0f)		cameraPitch_	+= XM_2PI;
	while (cameraRoll_ < 0.0f)		cameraRoll_		+= XM_2PI;

	XMVECTOR cameraPosition;
	XMVECTOR cameraTarget;
	XMVECTOR cameraRight;
	XMVECTOR cameraForward;
	XMVECTOR cameraUp;

	// Yaw (rotation around the Y axis) will have an impact on the forward and right vectors
	XMMATRIX cameraRotationYaw = XMMatrixRotationAxis(defaultUp, cameraYaw_);
	cameraRight = XMVector3TransformCoord(defaultRight, cameraRotationYaw);
	cameraForward = XMVector3TransformCoord(defaultForward, cameraRotationYaw);

	// Pitch (rotation around the X axis) impact the up and forward vectors
	XMMATRIX cameraRotationPitch = XMMatrixRotationAxis(cameraRight, cameraPitch_);
	cameraUp = XMVector3TransformCoord(defaultUp, cameraRotationPitch);
	cameraForward = XMVector3TransformCoord(cameraForward, cameraRotationPitch);

	// Roll (rotation around the Z axis) will impact the Up and Right vectors
	XMMATRIX cameraRotationRoll = XMMatrixRotationAxis(cameraForward, cameraRoll_);
	cameraUp = XMVector3TransformCoord(cameraUp, cameraRotationRoll);
	cameraRight = XMVector3TransformCoord(cameraRight, cameraRotationRoll);

	cameraPosition = XMLoadFloat4(&cameraPosition_);

	if (player_ != nullptr) {

		float cameraSnapLow = XM_PIDIV4 * 0.5f;
		float cameraSnapHigh = XM_PIDIV4 * 7;
		float cameraMidpoint = (cameraSnapHigh + cameraSnapLow) / 2.0f;

		// do some extra snapping
		if (cameraPitch_ > cameraSnapLow && cameraPitch_ < cameraMidpoint) {
			cameraPitch_ = cameraSnapLow;
		}

		if (cameraPitch_ >= cameraMidpoint && cameraPitch_ < cameraSnapHigh) {
			cameraPitch_ = cameraSnapHigh;
		}

		XMVECTOR offset = XMLoadFloat3(&offset_);
		XMMATRIX cameraRotation = XMMatrixRotationRollPitchYaw(0.0f, cameraYaw_, cameraPitch_);

		cameraUp = XMVector3TransformCoord(defaultUp, cameraRotation);
		XMVECTOR rotatedOffset = XMVector3Transform(offset, cameraRotation);

		XMFLOAT3 playerPosition;
		XMFLOAT3 cameraOffset;
		XMStoreFloat3(&playerPosition, player_->GetTransform()->GetPosition());
		XMStoreFloat3(&cameraOffset, rotatedOffset);

		cameraPosition = {
			playerPosition.x + cameraOffset.x,
			playerPosition.y + cameraOffset.y,
			playerPosition.z + cameraOffset.z,
			0.0
		};
	}
	else {
		// Adjust the camera position by the appropriate amount forward/back and left/right
		cameraPosition += moveLeftRight_ * cameraRight + moveForwardBack_ * cameraForward;
	}

	XMStoreFloat4(&cameraPosition_, cameraPosition);

	// Reset the amount we are moving
	moveLeftRight_ = 0.0f;
	moveForwardBack_ = 0.0f;

	// Calculate a vector that tells us the direction the camera is looking in
	cameraTarget = (player_ == nullptr)
		? cameraPosition + XMVector3Normalize(cameraForward)
		: player_->GetTransform()->GetPosition();

	// apply our y offset
	cameraPosition_.y += yMod_;
	cameraTarget += XMVectorSet(0.0f, yMod_, 0.0f, 0.0f);

	if (terrain_ != nullptr) {
		float yMin = terrain_->GetHeightAtPoint(cameraPosition_.x, cameraPosition_.z) + 8.0f;
		if (cameraPosition_.y < yMin) {
			float diff = yMin - cameraPosition_.y;
			cameraPosition_.y = yMin;
			cameraPosition += XMVectorSet(0.0f, diff, 0.0f, 0.0f);
		}
	}

	// and calculate our view matrix
	if (player_ == nullptr) {
		XMStoreFloat4x4(&viewMatrix_, XMMatrixLookAtLH(cameraPosition, cameraTarget, cameraUp));
	}
	else {
		XMStoreFloat4x4(&viewMatrix_, XMMatrixLookAtLH(cameraPosition, cameraTarget, cameraUp));
	}

}
