#pragma once
#include "core.h"
#include "DirectXCore.h"
#include <vector>

// Core material class.  Ideally, this should be extended to include more material attributes that can be
// recovered from Assimp, but this handles the basics.

class Material
{
public:
	Material(std::wstring materialName, XMFLOAT4 diffuseColour, XMFLOAT4 specularColour, float shininess, float opacity, ComPtr<ID3D11ShaderResourceView> texture );
	~Material();

	inline std::wstring						GetMaterialName() { return materialName_;  }
	inline XMFLOAT4							GetDiffuseColour() { return diffuseColour_; }
	inline XMFLOAT4							GetSpecularColour() { return specularColour_; }
	inline float							GetShininess() { return shininess_; }
	inline float							GetOpacity() { return opacity_; }
	inline ComPtr<ID3D11ShaderResourceView>	GetTexture() { return texture_; }

private:
	std::wstring							materialName_;
	XMFLOAT4								diffuseColour_;
	XMFLOAT4								specularColour_;
	float									shininess_;
	float									opacity_;
    ComPtr<ID3D11ShaderResourceView>		texture_;
};

// Basic SubMesh class.  A Mesh consists of one or more sub-meshes.  The submesh provides everything that is needed to
// draw the sub-mesh.

class SubMesh
{
public:
	SubMesh(ComPtr<ID3D11Buffer> vertexBuffer,
		ComPtr<ID3D11Buffer> indexBuffer,
		size_t vertexCount,
		size_t indexCount,
		std::shared_ptr<Material> material);
	~SubMesh();

	inline ComPtr<ID3D11Buffer>				GetVertexBuffer() { return vertexBuffer_; }
	inline ComPtr<ID3D11Buffer>				GetIndexBuffer() { return indexBuffer_; }
	inline std::shared_ptr<Material>		GetMaterial() { return material_; }
	inline size_t							GetVertexCount() { return vertexCount_; }
	inline size_t							GetIndexCount() { return indexCount_; }

private:
   	ComPtr<ID3D11Buffer>					vertexBuffer_;
	ComPtr<ID3D11Buffer>					indexBuffer_;
	std::shared_ptr<Material>				material_;
	size_t									vertexCount_;
	size_t									indexCount_;
};

// The core Mesh class.  A Mesh corresponds to a scene in ASSIMP. A mesh consists of one or more sub-meshes.

class Node
{
public	:
	inline void								SetName(std::wstring name) { name_ = name; }
	inline std::wstring						GetName() { return name_; }
	inline size_t							GetMeshCount() { return meshIndices_.size(); }
	inline unsigned int						GetMesh(unsigned int index) { return meshIndices_[index]; }
	inline void								AddMesh(unsigned int meshIndex) { meshIndices_.push_back(meshIndex); }
	inline size_t							GetChildrenCount() { return children_.size(); }
	inline std::shared_ptr<Node>			GetChild(unsigned int index) { return children_[index]; }
	inline void								AddChild(std::shared_ptr<Node> node) { children_.push_back(node); }

private:
	std::wstring							name_;
	std::vector<unsigned int>				meshIndices_;
	std::vector<std::shared_ptr<Node>>		children_;
};

class Mesh
{
public:
	size_t									GetSubMeshCount();
	std::shared_ptr<SubMesh>				GetSubMesh(unsigned int i);
	void									AddSubMesh(std::shared_ptr<SubMesh> subMesh);
	std::shared_ptr<Node>					GetRootNode();
	void									SetRootNode(std::shared_ptr<Node> node);

private:
	std::vector<std::shared_ptr<SubMesh>>	subMeshList_;
	std::shared_ptr<Node>					rootNode_;
};


