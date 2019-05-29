#pragma once
#include "Mesh.h"
#include "Renderer.h"
#include <map>
#include <assimp\importer.hpp>
#include <assimp\scene.h>
#include <assimp\postprocess.h>

struct VERTEX
{
	XMFLOAT3 Position;
	XMFLOAT3 Normal;
	XMFLOAT2 TexCoord;
};

struct MeshResourceStruct
{
	unsigned int				ReferenceCount;
	std::shared_ptr<Mesh>		MeshPointer;
};

typedef std::map<std::wstring, MeshResourceStruct>			MeshResourceMap;

struct MaterialResourceStruct
{
	unsigned int				ReferenceCount;
	std::shared_ptr<Material>	MaterialPointer;
};

typedef std::map<std::wstring, MaterialResourceStruct>		MaterialResourceMap;

typedef std::map<std::wstring, std::shared_ptr<Renderer>>	RendererResourceMap;

class ResourceManager
{
public:
	ResourceManager();
	~ResourceManager();
				
	std::shared_ptr<Renderer>					GetRenderer(std::wstring rendererName);

	std::shared_ptr<Mesh>						GetMesh(std::wstring modelName);
	void										ReleaseMesh(std::wstring modelName);

	void										CreateMaterialFromTexture(std::wstring textureName);
    void										CreateMaterialWithNoTexture(std::wstring materialName, XMFLOAT4 diffuseColour, XMFLOAT4 specularColour, float shininess, float opacity);
    void										CreateMaterial(std::wstring materialName, XMFLOAT4 diffuseColour, XMFLOAT4 specularColour, float shininess, float opacity, std::wstring textureName);
	std::shared_ptr<Material>					GetMaterial(std::wstring materialName);
	void										ReleaseMaterial(std::wstring materialName);

private:
	MeshResourceMap								meshResources_;
	MaterialResourceMap							materialResources_;
	RendererResourceMap							rendererResources_;

	ComPtr<ID3D11Device>						device_;
	ComPtr<ID3D11DeviceContext>					deviceContext_;

	ComPtr<ID3D11ShaderResourceView>			defaultTexture_;
    
	std::shared_ptr<Node>						CreateNodes(aiNode * sceneNode);
	std::shared_ptr<Mesh>						LoadModelFromFile(std::wstring modelName);
    void										InitialiseMaterial(std::wstring materialName, XMFLOAT4 diffuseColour, XMFLOAT4 specularColour, float shininess, float opacity, std::wstring textureName);
};

