#pragma once
#include "MeshNode.h"
#include <vector>

struct PlayerControlState {
	float MoveX;
	float MoveZ;
	float HorizontalSpeed;
	bool IsJumping;
};

class PlayerNode :
	public MeshNode
{
public:
	PlayerNode(std::wstring name, std::wstring modelName);

	void SetActive(bool active)
		{ active_ = active; }

	bool IsActive()
		{ return active_; }

	void SetTerrain(std::shared_ptr<TerrainNode> terrain)
		{ terrain_ = terrain; };

	void SetControlState(PlayerControlState state)
		{ currentState_ = state; };

	void Start();
	void Update(FXMMATRIX& currentWorldTransformation);

	bool IsGrounded();

private:
	float							yVelocity_			= 0.0f;

	bool							active_;
	PlayerControlState				currentState_;

	std::vector<bool>				groundedLog_;
	std::shared_ptr<TerrainNode>	terrain_;
};

