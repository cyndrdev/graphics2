#pragma once
#include "SceneNode.h"
#include "GameConstants.h"
#include "ResourceManager.h"
#include <vector>
#include <array>

struct TERRAIN_VERTEX {
	DirectX::XMFLOAT3 Position;
	DirectX::XMFLOAT3 Normal;
	DirectX::XMFLOAT2 TexCoord;
	DirectX::XMFLOAT2 BlendMapTexCoord;
};

class TerrainNode
	: virtual public SceneNode
{
public:
	TerrainNode(std::wstring name);
	~TerrainNode();

	bool Initialise(void);
	void Start(void);
	void Render(void);
	void Update(DirectX::FXMMATRIX& currentWorldTransformation);
	void Shutdown(void);
	void SetWorldTransform(DirectX::FXMMATRIX& worldTransformation) { XMStoreFloat4x4(&worldTransformation_, worldTransformation); }
	float GetHeightAtPoint(float x, float z);

private:
	bool LoadHeightMap(std::wstring filename);
	void LoadTerrainTextures(void);
	void GenerateBlendMap(void);
	void BuildTerrainData(void);
	void CalculateTerrainNormals(void);
	void BuildGeometryBuffers(void);
	void BuildVertexLayout(void);
	void BuildShaders(void);
	void BuildConstantBuffer(void);
	void BuildRendererStates(void);

	std::vector<float>									heightmap_;
	std::array<TERRAIN_VERTEX, VERTEX_TARGET>			vertices_;
	std::array<UINT, INDEX_TARGET>						indices_;

	Microsoft::WRL::ComPtr<ID3D11Device>				device_;
	Microsoft::WRL::ComPtr<ID3D11DeviceContext>			deviceContext_;

	Microsoft::WRL::ComPtr<ID3D11Buffer>				vertexBuffer_;
	Microsoft::WRL::ComPtr<ID3D11Buffer>				indexBuffer_;

	Microsoft::WRL::ComPtr<ID3DBlob>					vertexShaderByteCode_ = nullptr;
	Microsoft::WRL::ComPtr<ID3D11VertexShader>			vertexShader_;
	Microsoft::WRL::ComPtr<ID3DBlob>					pixelShaderByteCode_ = nullptr;
	Microsoft::WRL::ComPtr<ID3D11PixelShader>			pixelShader_;

	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>	texturesResourceView_;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>	blendMapResourceView_;

	Microsoft::WRL::ComPtr<ID3D11InputLayout>			layout_;

	Microsoft::WRL::ComPtr<ID3D11Buffer>				constantBuffer_;

	Microsoft::WRL::ComPtr<ID3D11RasterizerState>		defaultRasteriserState_;
	Microsoft::WRL::ComPtr<ID3D11RasterizerState>		wireframeRasteriserState_;
};

