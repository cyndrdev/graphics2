#pragma once
#include "DirectXFramework.h"
#include "TerrainNode.h"
#include "PlayerNode.h"

class Graphics2 : public DirectXFramework
{
public:
	Graphics2();
	void CreateSceneGraph();
	void UpdateSceneGraph();
private:
	bool firstFrame_ = true;
	float a_;

	std::shared_ptr<TerrainNode> terrain_;
	std::shared_ptr<PlayerNode> player_;
};

