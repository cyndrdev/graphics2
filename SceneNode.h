#pragma once
#include "core.h"
#include "DirectXCore.h"
#include "Transform.h"
#include "Collider.h"

// Abstract base class for all nodes of the scene graph.  
// This scene graph implements the Composite Design Pattern

class SceneNode;

typedef std::shared_ptr<SceneNode>	SceneNodePointer;

class SceneNode : public std::enable_shared_from_this<SceneNode>
{
public:
	SceneNode(std::wstring name) {
		name_ = name; 
		transform_ = std::make_shared<Transform>();

		// by default, nodes don't have colliders
		collider_ = nullptr;
		XMStoreFloat4x4(&worldTransformation_, XMMatrixIdentity()); 

	};
	~SceneNode(void) {};

	// core methods
	virtual bool Initialise() = 0;
	virtual void Start() = 0;
	virtual void Update(FXMMATRIX& currentWorldTransformation);

	virtual void CreateCollider(float height, float radius, XMFLOAT3 offset, bool pushable);

	virtual void Render() = 0;
	virtual void Shutdown() = 0;

	std::shared_ptr<Transform>	GetTransform()	{ return transform_; }
	std::shared_ptr<Collider>	GetCollider()	{ return collider_; }
		
	virtual void Add(SceneNodePointer node) {};
	virtual void Remove(SceneNodePointer node) {};
	virtual	SceneNodePointer Find(std::wstring name) { return (name_ == name) ? shared_from_this() : nullptr; };
	virtual std::wstring GetName() { return name_; }

protected:
	XMFLOAT4X4					worldTransformation_;
	XMFLOAT4X4					combinedWorldTransformation_;
	std::shared_ptr<Transform>	transform_;
	std::shared_ptr<Collider>	collider_;
	std::wstring				name_;
};
