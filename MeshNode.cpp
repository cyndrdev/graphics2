#include "MeshNode.h"

bool MeshNode::Initialise()
{
	resourceManager_ = DirectXFramework::GetDXFramework()->GetResourceManager();
	renderer_ = std::dynamic_pointer_cast<MeshRenderer>(resourceManager_->GetRenderer(L"PNT"));
	mesh_ = resourceManager_->GetMesh(modelName_);
	if (mesh_ == nullptr)
	{
		return false;
	}
	return renderer_->Initialise();
}

void MeshNode::Start()
{
}

void MeshNode::Shutdown()
{
	resourceManager_->ReleaseMesh(modelName_);
}

void MeshNode::Render()
{
	// grab useful references
	std::shared_ptr<Camera> camera = DirectXFramework::GetDXFramework()->GetCamera();
	std::shared_ptr<Lighting> lighting = DirectXFramework::GetDXFramework()->GetLighting();

	XMFLOAT4 cameraPosition;
	XMStoreFloat4(&cameraPosition, camera->GetCameraPosition());

	XMVECTOR lightDirection = XMLoadFloat4(&lighting->DirectionalLightVector);

	renderer_->SetMesh(mesh_);
	renderer_->SetWorldTransformation(XMLoadFloat4x4(&combinedWorldTransformation_));
	renderer_->SetCameraPosition(cameraPosition);
	renderer_->SetAmbientLight(lighting->AmbientLight);
	renderer_->SetDirectionalLight(
		XMVector3InverseRotate(lightDirection, GetTransform()->GetRotation()),
		lighting->DirectionalLightColor
	);

	renderer_->Render();
}

