#pragma once
#include "SceneNode.h"
#include "DirectXFramework.h"
#include "MeshRenderer.h"

class MeshNode : public SceneNode
{
public:
public:
	MeshNode(std::wstring name, std::wstring modelName) : SceneNode(name) { modelName_ = modelName; }

	bool Initialise();
	void Start();
	void Render();
	void Shutdown();

private:
	std::shared_ptr<MeshRenderer>		renderer_;

	std::wstring						modelName_;
	std::shared_ptr<ResourceManager>	resourceManager_;
	std::shared_ptr<Mesh>				mesh_;
};

