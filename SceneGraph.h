#pragma once
#include <vector>
#include "SceneNode.h"

class SceneGraph;

typedef std::shared_ptr<SceneGraph> SceneGraphPointer;

class SceneGraph : public SceneNode
{
public:
	SceneGraph() : SceneNode(L"Root") {};
	SceneGraph(std::wstring name) : SceneNode(name) {};
	~SceneGraph(void) {};

	SceneNodePointer	operator[](const size_t index) const;

	virtual bool		Initialise(void);
	virtual void		Start(void);
	virtual void		Update(FXMMATRIX& currentWorldTransformation);
	virtual void		Render(void);
	virtual void		Shutdown(void);

	void				Add(SceneNodePointer node);
	void				Remove(SceneNodePointer node);
	SceneNodePointer	Find(std::wstring name);

	size_t				GetChildCount()			const;
	SceneNodePointer	GetChild(size_t index)	const;

private:
	std::vector<SceneNodePointer> children_;
};
