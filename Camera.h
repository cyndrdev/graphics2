#pragma once
#include "core.h"
#include "DirectXCore.h"
#include "SceneNode.h"
#include "TerrainNode.h"

class Camera
{
public:
    Camera();
    ~Camera();

    void Update();
    XMMATRIX GetViewMatrix();
    XMVECTOR GetCameraPosition();

	void FollowNode(SceneNodePointer node, float offset, float y_mod);
    void SetCameraPosition(float x, float y, float z);
    void SetPitch(float pitch);
	void SetTotalPitch(float pitch);
	float GetPitch() const;
    void SetYaw(float yaw);
	void SetTotalYaw(float yaw);
	float GetYaw() const;
	void SetRoll(float roll);
	void SetTotalRoll(float roll);
	float GetRoll() const;
	void SetLeftRight(float leftRight);
    void SetForwardBack(float forwardBack);
	void SetRelativeY(float relY);

	void SetTerrain(std::shared_ptr<TerrainNode> terrain)
	{
		terrain_ = terrain;
	};

	bool IsFollowingNode();

private:
    XMFLOAT4			cameraPosition_;

    XMFLOAT4X4			viewMatrix_;

    float				moveLeftRight_;
    float				moveForwardBack_;

    float				cameraYaw_;
    float				cameraPitch_;
    float				cameraRoll_;

	SceneNodePointer	player_ = nullptr;
	std::shared_ptr<TerrainNode> terrain_ = nullptr;
	XMFLOAT3			offset_;
	float				yMod_;
};

