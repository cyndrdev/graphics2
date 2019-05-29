#pragma once
#include "SceneGraph.h"

SceneNodePointer SceneGraph::operator[](const size_t index) const
{
	return children_[index];
}

bool SceneGraph::Initialise(void)
{
	bool allSuccessfull = true;
	bool success;

	for (auto&& child : children_) {
		success = child->Initialise();
		allSuccessfull &= success;
	}

	return allSuccessfull;
}

void SceneGraph::Start(void)
{
	for (auto&& child : children_) {
		child->Start();
	}
}

void SceneGraph::Update(FXMMATRIX & currentWorldTransformation)
{
	for (auto&& child : children_) {
		child->Update(currentWorldTransformation);
	}
}

void SceneGraph::Render(void)
{
	for (auto&& child : children_) {
		child->Render();
	}
}

void SceneGraph::Shutdown(void)
{
	for (auto&& child : children_) {
		child->Shutdown();
	}
}

void SceneGraph::Add(SceneNodePointer node)
{
	children_.push_back(node);
}

void SceneGraph::Remove(SceneNodePointer node)
{
	for (int i = 0; i < children_.size(); i++) {
		children_[i]->Remove(node);

		if (children_[i] == node) {
			children_.erase(children_.begin() + i);
			return;
		}
	}
}

SceneNodePointer SceneGraph::Find(std::wstring name)
{
	SceneNodePointer foundNode;

	for (auto&& child : children_) {
		foundNode = child->Find(name);

		if (foundNode != nullptr) {
			return foundNode;
		}
	}

	return nullptr;
}

size_t SceneGraph::GetChildCount() const
{
	return children_.size();
}

SceneNodePointer SceneGraph::GetChild(size_t index) const
{
	return children_[index];
}
