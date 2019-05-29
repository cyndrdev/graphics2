#include "SceneNode.h"

void SceneNode::CreateCollider(float height, float radius, XMFLOAT3 offset, bool pushable)
{
	collider_ = std::make_shared<Collider>(height, radius, offset, pushable);
}

void SceneNode::Update(FXMMATRIX& currentWorldTransformation) {
	XMStoreFloat4x4(&combinedWorldTransformation_, transform_->GetWorldTransform() * currentWorldTransformation);
	if (collider_ != nullptr) {
		XMFLOAT3 worldPosition;
		XMStoreFloat3(&worldPosition, transform_->GetPosition());
		collider_->SetWorldPosition(worldPosition);
	}
};
