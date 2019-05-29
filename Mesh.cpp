#include "Mesh.h"

// Material methods

Material::Material(std::wstring materialName, XMFLOAT4 diffuseColour, XMFLOAT4 specularColour, float shininess, float opacity, ComPtr<ID3D11ShaderResourceView> texture )
{
	materialName_ =		materialName;
	diffuseColour_ =	diffuseColour;
	specularColour_ =	specularColour;
	shininess_ =		shininess;
	opacity_ =			opacity;
    texture_ =			texture;
}

Material::~Material(void)
{
}

// SubMesh methods

SubMesh::SubMesh(
		ComPtr<ID3D11Buffer> vertexBuffer,
		ComPtr<ID3D11Buffer> indexBuffer,
		size_t vertexCount,
		size_t indexCount,
		std::shared_ptr<Material> material
)
{
	vertexBuffer_ =		vertexBuffer;
	indexBuffer_ =		indexBuffer;
	vertexCount_ =		vertexCount;
	indexCount_ =		indexCount;
	material_ =			material;
}

SubMesh::~SubMesh(void)
{
}

// Mesh methods

size_t Mesh::GetSubMeshCount()
{
	return subMeshList_.size();
}

std::shared_ptr<SubMesh> Mesh::GetSubMesh(unsigned int i)
{
	if (i >= 0 && i < subMeshList_.size())
	{
		return subMeshList_[i];
	}
	return nullptr;
}

void Mesh::AddSubMesh(std::shared_ptr<SubMesh> subMesh)
{
	subMeshList_.push_back(subMesh);
}

std::shared_ptr<Node> Mesh::GetRootNode()
{
	return rootNode_;
}

void Mesh::SetRootNode(std::shared_ptr<Node> node)
{
	rootNode_ = node;
}
