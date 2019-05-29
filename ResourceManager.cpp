#define _SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING
#define _SILENCE_ALL_CXX17_DEPRECATION_WARNINGS

#include "ResourceManager.h"
#include "DirectXFramework.h"
#include <sstream>
#include "WICTextureLoader.h"
#include <locale>
#include <codecvt>
#include "MeshRenderer.h"

#pragma comment(lib, "../Assimp/lib/release/assimp-vc140-mt.lib")

using namespace Assimp;

std::wstring s2ws(const std::string& str)
{
	using convert_typeX = std::codecvt_utf8<wchar_t>;
	std::wstring_convert<convert_typeX, wchar_t> converterX;

	return converterX.from_bytes(str);
}

std::string ws2s(const std::wstring& wstr)
{
	using convert_typeX = std::codecvt_utf8<wchar_t>;
	std::wstring_convert<convert_typeX, wchar_t> converterX;

	return converterX.to_bytes(wstr);
}

//-------------------------------------------------------------------------------------------

ResourceManager::ResourceManager()
{
	device_ = DirectXFramework::GetDXFramework()->GetDevice();
	deviceContext_ = DirectXFramework::GetDXFramework()->GetDeviceContext();

	// create our default texture
	HRESULT result
		= CreateWICTextureFromFile(
			device_.Get(),
			deviceContext_.Get(),
			L"white.png",
			nullptr,
			defaultTexture_.GetAddressOf()
		);

	if (FAILED(result))
		defaultTexture_ = nullptr;
}

ResourceManager::~ResourceManager(void)
{
}

std::shared_ptr<Renderer> ResourceManager::GetRenderer(std::wstring rendererName)
{
	RendererResourceMap::iterator it = rendererResources_.find(rendererName);
	if (it != rendererResources_.end())
	{
		return it->second;
	}
	else
	{
		if (rendererName == L"PNT")
		{
			std::shared_ptr<Renderer> renderer = std::make_shared<MeshRenderer>();
			rendererResources_[rendererName] = renderer;
			return renderer;
		}
	}
	return nullptr;
}

std::shared_ptr<Mesh> ResourceManager::GetMesh(std::wstring modelName)
{
	// check if the mesh is cached
	MeshResourceMap::iterator it = meshResources_.find(modelName);
	if (it != meshResources_.end())
	{
		// update reference count and return pointer to the mesh
		it->second.ReferenceCount++;
		return it->second.MeshPointer;
	}
	else
	{
		// we need to load the mesh file
		std::wcout << L"loading mesh " << modelName << L"...\t";
		std::shared_ptr<Mesh> mesh = LoadModelFromFile(modelName);

		if (mesh != nullptr)
		{
			MeshResourceStruct resourceStruct;
			resourceStruct.ReferenceCount = 1;
			resourceStruct.MeshPointer = mesh;
			meshResources_[modelName] = resourceStruct;

			std::cout << "done." << std::endl;
			return mesh;
		}
		else
		{
			std::cout << "fail." << std::endl;
			return nullptr;
		}
	}
}

void ResourceManager::ReleaseMesh(std::wstring modelName)
{
	MeshResourceMap::iterator it = meshResources_.find(modelName);
	if (it != meshResources_.end())
	{
		it->second.ReferenceCount--;
		if (it->second.ReferenceCount == 0)
		{
			// release any materials used by this mesh
			std::shared_ptr<Mesh> mesh = it->second.MeshPointer;
			unsigned int subMeshCount = static_cast<unsigned int>(mesh->GetSubMeshCount());

			// loop through all submeshes in the mesh
			for (unsigned int i = 0; i < subMeshCount; i++)
			{
				std::shared_ptr<SubMesh> subMesh = mesh->GetSubMesh(i);
				std::wstring materialName = subMesh->GetMaterial()->GetMaterialName();
				ReleaseMaterial(materialName);
			}

			// if no other nodes are using this mesh, remove it from the map
			// this should also release the resources
			it->second.MeshPointer = nullptr;
			meshResources_.erase(modelName);
		}
	}
}

void ResourceManager::CreateMaterialFromTexture(std::wstring textureName)
{
    return InitialiseMaterial(
		textureName, 
		XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), 
		XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f),
		0,
		1.0f,
		textureName
	);
}

void ResourceManager::CreateMaterialWithNoTexture(std::wstring materialName, XMFLOAT4 diffuseColour, XMFLOAT4 specularColour, float shininess, float opacity)
{
    return InitialiseMaterial(materialName, diffuseColour, specularColour, shininess, opacity, L"");
}

void ResourceManager::CreateMaterial(std::wstring materialName, XMFLOAT4 diffuseColour, XMFLOAT4 specularColour, float shininess, float opacity, std::wstring textureName)
{
    return InitialiseMaterial(materialName, diffuseColour, specularColour, shininess, opacity, textureName);
}

std::shared_ptr<Material> ResourceManager::GetMaterial(std::wstring materialName)
{
	MaterialResourceMap::iterator it = materialResources_.find(materialName);
	if (it != materialResources_.end())
	{
		it->second.ReferenceCount++;
		return it->second.MaterialPointer;
	}
	return nullptr;
}

void ResourceManager::ReleaseMaterial(std::wstring materialName)
{
	MaterialResourceMap::iterator it = materialResources_.find(materialName);
	if (it != materialResources_.end())
	{
		it->second.ReferenceCount--;
		if (it->second.ReferenceCount == 0)
		{
			it->second.MaterialPointer = nullptr;
			meshResources_.erase(materialName);
		}
	}
}

void ResourceManager::InitialiseMaterial(std::wstring materialName, XMFLOAT4 diffuseColour, XMFLOAT4 specularColour, float shininess, float opacity, std::wstring textureName)
{
	MaterialResourceMap::iterator it = materialResources_.find(materialName);
	if (it == materialResources_.end())
	{
		// we are creating the material for the first time
		ComPtr<ID3D11ShaderResourceView> texture;
		if (textureName.size() > 0)
		{
			// try to load the texture
			HRESULT result
				= CreateWICTextureFromFile(
					device_.Get(),
					deviceContext_.Get(),
					textureName.c_str(),
					nullptr,
					texture.GetAddressOf()
				);

			if (FAILED(result))
			{
				// if we can't load the texture, then just use the default.
				texture = defaultTexture_;
			}
		}
		else
		{
			texture = defaultTexture_;
		}
		std::shared_ptr<Material> material = std::make_shared<Material>(materialName, diffuseColour, specularColour, shininess, opacity, texture);
		MaterialResourceStruct resourceStruct;
		resourceStruct.ReferenceCount = 0;
		resourceStruct.MaterialPointer = material;
		materialResources_[materialName] = resourceStruct;
	}
}

std::shared_ptr<Node> ResourceManager::CreateNodes(aiNode * sceneNode)
{
	std::shared_ptr<Node> node = std::make_shared<Node>();
	node->SetName(s2ws(std::string(sceneNode->mName.C_Str())));

	// get the meshes associated with this node
	unsigned int meshCount = sceneNode->mNumMeshes;
	for (unsigned int i = 0; i < meshCount; i++)
	{
		node->AddMesh(sceneNode->mMeshes[i]);
	}

	// now process the children of this node
	unsigned int childrenCount = sceneNode->mNumChildren;
	for (unsigned int i = 0; i < childrenCount; i++)
	{
		node->AddChild(CreateNodes(sceneNode->mChildren[i]));
	}
	return node;
}

std::shared_ptr<Mesh> ResourceManager::LoadModelFromFile(std::wstring modelName)
{
	ComPtr<ID3D11Buffer> vertexBuffer;
	ComPtr<ID3D11Buffer> indexBuffer;
    std::wstring * materials = nullptr;
	
	Importer importer;

	unsigned int postProcessSteps = aiProcess_Triangulate |
		                            aiProcess_ConvertToLeftHanded;
	std::string modelNameUTF8 = ws2s(modelName);
	const aiScene * scene = importer.ReadFile(modelNameUTF8.c_str(), postProcessSteps);

	if (!scene || !scene->HasMeshes())
	{
        // if failed to load, or if there are
        // no meshes, then there is nothing to do.
		return nullptr;
	}
    if (scene->HasMaterials())
    {
		// dirty hack to get directory
        std::string::size_type slashIndex = modelNameUTF8.find_last_of("\\");
        std::string directory;
        if (slashIndex == std::string::npos) 
        {
            directory = ".";
        }
        else if (slashIndex == 0) 
        {
            directory = "/";
        }
        else 
        {
            directory = modelNameUTF8.substr(0, slashIndex);
        }

        // deal with the materials/textures first
        materials = new std::wstring[scene->mNumMaterials];
        for (unsigned int i = 0; i < scene->mNumMaterials; i++)
        {
            // get the core material properties. 
            aiMaterial * material = scene->mMaterials[i];

			aiColor3D &defaultColour = aiColor3D(0.0f, 0.0f, 0.0f);
			aiColor3D& diffuseColour = aiColor3D(0.0f, 0.0f, 0.0f);
			if (material->Get(AI_MATKEY_COLOR_DIFFUSE, diffuseColour) != aiReturn_SUCCESS)
			{
				diffuseColour = defaultColour;
			}
            aiColor3D& specularColour = aiColor3D(0.0f, 0.0f, 0.0f);
			if (material->Get(AI_MATKEY_COLOR_SPECULAR, specularColour) != aiReturn_SUCCESS)
			{
				specularColour = defaultColour;
			}
            float defaultShininess = 0.0f;
            float& shininess = defaultShininess;
            material->Get(AI_MATKEY_SHININESS, shininess);
			float defaultOpacity = 1.0f;
			float& opacity = defaultOpacity;
			material->Get(AI_MATKEY_OPACITY, opacity);
			bool defaultTwoSided = false;
			bool& twoSided = defaultTwoSided;
			material->Get(AI_MATKEY_TWOSIDED, twoSided);
			std::string fullTextureNamePath = "";
            if (material->GetTextureCount(aiTextureType_DIFFUSE) > 0)
            {
                aiString textureName;
				float blendFactor;
				aiTextureOp blendOp;
                if (material->GetTexture(aiTextureType_DIFFUSE, 0, &textureName, NULL, NULL, &blendFactor, &blendOp, NULL) == AI_SUCCESS)
                {
                    // get full path to texture by prepending the same folder as included in the model name.
                    // assumes that textures are in the same folder as the model files (not ideal)
                    fullTextureNamePath = directory + "\\" + textureName.data;
                }
            }

            // create a unique name for the material
            std::stringstream materialNameStream;
            materialNameStream << modelNameUTF8 << i;
            std::string materialName = materialNameStream.str();
			std::wstring materialNameWS = s2ws(materialName);

			CreateMaterial(
				materialNameWS,
				XMFLOAT4(diffuseColour.r, diffuseColour.g, diffuseColour.b, 1.0f),
				XMFLOAT4(specularColour.r, specularColour.g, specularColour.b, 1.0f),
				shininess,
				opacity, 
				s2ws(fullTextureNamePath)
			);

            materials[i] = materialNameWS;
        }
    }

	// === build up mesh === //
	std::shared_ptr<Mesh> resourceMesh = std::make_shared<Mesh>();

    for (unsigned int sm = 0; sm < scene->mNumMeshes; sm++)
    {
	    aiMesh * subMesh = scene->mMeshes[sm];
	    unsigned int numVertices = subMesh->mNumVertices;
	    bool hasNormals = subMesh->HasNormals();
	    bool hasTexCoords = subMesh->HasTextureCoords(0);

	    if (numVertices == 0 || !hasNormals)
		    return nullptr;

	    // build up our vertex structure
	    aiVector3D * subMeshVertices = subMesh->mVertices;
	    aiVector3D * subMeshNormals = subMesh->mNormals;

		// handle uv coords
	    aiVector3D * subMeshTexCoords = subMesh->mTextureCoords[0];

	    VERTEX * modelVertices = new VERTEX[numVertices];
	    VERTEX * currentVertex = modelVertices;

	    for (unsigned int i = 0; i < numVertices; i++)
	    {
			currentVertex->Position = XMFLOAT3(subMeshVertices->x, subMeshVertices->y, subMeshVertices->z);
			currentVertex->Normal = XMFLOAT3(subMeshNormals->x, subMeshNormals->y, subMeshNormals->z);

		    subMeshVertices++;
		    subMeshNormals++;

            if (!hasTexCoords)
            {
				currentVertex->TexCoord = XMFLOAT2(0.0f, 0.0f);
            }
            else
            {
                // handle negative texture coordinates by wrapping them to positive. 
		        if (subMeshTexCoords->x < 0)
		        {
			        currentVertex->TexCoord.x = subMeshTexCoords->x + 1.0f;
		        }
		        else
		        {
			        currentVertex->TexCoord.x = subMeshTexCoords->x;
		        }
		        if (subMeshTexCoords->y < 0)
		        {
			        currentVertex->TexCoord.y = subMeshTexCoords->y + 1.0f;
		        }
		        else
		        {
			        currentVertex->TexCoord.y = subMeshTexCoords->y;
		        }

		        subMeshTexCoords++;
            }
		    currentVertex++;
	    }

		D3D11_BUFFER_DESC vertexBufferDescriptor;

		vertexBufferDescriptor.Usage =					D3D11_USAGE_IMMUTABLE;
		vertexBufferDescriptor.ByteWidth =				sizeof(VERTEX) * numVertices;
		vertexBufferDescriptor.BindFlags =				D3D11_BIND_VERTEX_BUFFER;
		vertexBufferDescriptor.CPUAccessFlags =			0;
		vertexBufferDescriptor.MiscFlags =				0;
		vertexBufferDescriptor.StructureByteStride =	0;

		D3D11_SUBRESOURCE_DATA vertexInitialisationData;
		vertexInitialisationData.pSysMem = modelVertices;

		HRESULT result
			= device_->CreateBuffer(&vertexBufferDescriptor, &vertexInitialisationData, vertexBuffer.GetAddressOf());

		if (FAILED(result)) return nullptr;

		// extract indices
	    unsigned int numberOfFaces = subMesh->mNumFaces;
	    unsigned int numberOfIndices = numberOfFaces * 3;

	    aiFace * subMeshFaces = subMesh->mFaces;
	    if (subMeshFaces->mNumIndices != 3)
	    {
			// we can only handle tris for now
		    return nullptr;
	    }

	    unsigned int * modelIndices = new unsigned int[numberOfIndices * 3];
	    unsigned int * currentIndex  = modelIndices;

	    for (unsigned int i = 0; i < numberOfFaces; i++)
	    {
		    *currentIndex++ = subMeshFaces->mIndices[0];
		    *currentIndex++ = subMeshFaces->mIndices[1];
		    *currentIndex++ = subMeshFaces->mIndices[2];
		    subMeshFaces++;
	    }

		// create our index buffer
		D3D11_BUFFER_DESC indexBufferDescriptor;
		indexBufferDescriptor.Usage = D3D11_USAGE_IMMUTABLE;
		indexBufferDescriptor.ByteWidth = sizeof(UINT) * numberOfIndices * 3;
		indexBufferDescriptor.BindFlags = D3D11_BIND_INDEX_BUFFER;
		indexBufferDescriptor.CPUAccessFlags = 0;
		indexBufferDescriptor.MiscFlags = 0;
		indexBufferDescriptor.StructureByteStride = 0;

		D3D11_SUBRESOURCE_DATA indexInitialisationData;
		indexInitialisationData.pSysMem = modelIndices;

		result = device_->CreateBuffer(
			&indexBufferDescriptor, 
			&indexInitialisationData, 
			indexBuffer.GetAddressOf()
		);

		if (FAILED(result)) return nullptr;

		// do we have a material associated with this mesh?
		
		std::shared_ptr<Material> material = nullptr;
        if (scene->HasMaterials())
        {
            material = GetMaterial(materials[subMesh->mMaterialIndex]);
        }

	    std::shared_ptr<SubMesh> resourceSubMesh 
			= std::make_shared<SubMesh>(
				vertexBuffer, 
				indexBuffer, 
				numVertices, 
				numberOfIndices, 
				material
			);

	    resourceMesh->AddSubMesh(resourceSubMesh);
		delete[] modelVertices;
		delete[] modelIndices;
    }

	// build our hierarchy
	resourceMesh->SetRootNode(CreateNodes(scene->mRootNode));
	return resourceMesh;
}
