#include "PlayerNode.h"
#include "GameConstants.h"

PlayerNode::PlayerNode(std::wstring name, std::wstring modelName)
	: MeshNode(name, modelName)
{
}

void PlayerNode::Start()
{
	for (int i = 0; i < PLAYER_JUMP_LENIENCE_FRAMES; i++) {
		groundedLog_.push_back(true);
	}

	GetTransform()->SetPosition(-1370.0f, 20.0f, 540.0f);
}

void PlayerNode::Update(FXMMATRIX& currentWorldTransformation)
{
	SceneNode::Update(currentWorldTransformation);
	if (!active_) return;

	// take camera rotation into account
	XMVECTOR positionVector = GetTransform()->GetPosition();

	float yaw = XMConvertToRadians(
		DirectXFramework::GetDXFramework()->GetCamera()->GetYaw()
	);

	XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
	XMVECTOR right = XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f);
	XMVECTOR forward = XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);

	XMMATRIX camRotYaw = XMMatrixRotationAxis(up, yaw);

	forward = XMVector3TransformCoord(forward, camRotYaw);
	right = XMVector3TransformCoord(right, camRotYaw);

	// move our fox!
	// the fox is rotated, so we need to take that into account.
	// bad model is bad.
	positionVector -= currentState_.MoveX * forward * currentState_.HorizontalSpeed;
	positionVector += currentState_.MoveZ * right * currentState_.HorizontalSpeed;

	XMFLOAT3 position;
	XMStoreFloat3(&position, positionVector);

	// apply gravity
	yVelocity_ += FRAME_GRAVITY;
	position.y += yVelocity_;

	// keep within bounds
	float edge = ((GRID_SIZE - 1) * GRID_STEP * 0.5f) - CAMERA_DISTANCE;

	if (position.x > +edge)	position.x = +edge;
	if (position.x < -edge)	position.x = -edge;
	if (position.z > +edge)	position.z = +edge;
	if (position.z < -edge)	position.z = -edge;

	// make sure we don't sink through the ground
	float minY = terrain_->GetHeightAtPoint(position.x, position.z);

	if (minY > position.y) {
		position.y = minY;

		// we should reset velocity here to be accurate, but
		// i've found setting a slight negative velocity helps
		// make movement feel smoother. yay, game design!
		yVelocity_ = -1.0f;
	}

	// update our grounded state
	bool grounded = (minY + PLAYER_JUMP_LENIENCE_Y) >= position.y;

	groundedLog_.erase(groundedLog_.begin());
	groundedLog_.push_back(grounded);

	if (IsGrounded() && currentState_.IsJumping) {
		yVelocity_ = PLAYER_JUMP_POWER;
	}

	// set our position based on all we've done so far
	GetTransform()->SetPosition(position);

	// now let's calculate our rotation
	XMVECTOR pawOffset =	right * (PLAYER_SIZE / 2.0f);
	XMVECTOR frontPaws =	positionVector - pawOffset;
	XMVECTOR backPaws =		positionVector + pawOffset;

	float frontY = terrain_->GetHeightAtPoint(
		XMVectorGetX(frontPaws),
		XMVectorGetZ(frontPaws)
	);

	float backY = terrain_->GetHeightAtPoint(
		XMVectorGetX(backPaws),
		XMVectorGetZ(backPaws)
	);

	float climbAngle = atan((backY - frontY) / PLAYER_SIZE);

	GetTransform()->SetRotation(climbAngle, yaw - XM_PIDIV2, 0.0f);
}

bool PlayerNode::IsGrounded()
{
	return std::any_of(
		groundedLog_.begin(), 
		groundedLog_.end(), 
		[](bool b) { 
			return b; 
		}
	);
}
