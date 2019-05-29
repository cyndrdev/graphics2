#pragma once
#include "SceneNode.h"
#include "DirectXCore.h"

class SkyboxNode :
	virtual public SceneNode
{
public:
	SkyboxNode(std::wstring name, std::wstring boxTexturePath, float distance, int tesselation);
	~SkyboxNode();

	bool Initialise(void);
	void Start(void);
	void Update(FXMMATRIX& currentWorldTransformation);
	void Render(void);
	void Shutdown(void);

private:

	struct VERTEX {
		XMFLOAT3 Position;
	};

	void CreateSphere(float radius, size_t tessellation);
	void BuildGeometryBuffers(void);
	void BuildVertexLayout(void);
	void BuildShaders(void);
	void BuildConstantBuffer(void);
	void BuildTexture(void);
	void BuildRendererStates(void);
	void BuildDepthStencilState(void);

	VERTEX*												vertices_;
	UINT*												indices_;

	size_t												vertexCount_;
	size_t												indexCount_;

	float												radius_;
	int													tessellation_;
	std::wstring										texturePath_;

	Microsoft::WRL::ComPtr<ID3D11Device>				device_;
	Microsoft::WRL::ComPtr<ID3D11DeviceContext>			deviceContext_;

	Microsoft::WRL::ComPtr<ID3D11Buffer>				vertexBuffer_;
	Microsoft::WRL::ComPtr<ID3D11Buffer>				indexBuffer_;

	Microsoft::WRL::ComPtr<ID3DBlob>					vertexShaderByteCode_ = nullptr;
	Microsoft::WRL::ComPtr<ID3D11VertexShader>			vertexShader_;
	Microsoft::WRL::ComPtr<ID3DBlob>					pixelShaderByteCode_ = nullptr;
	Microsoft::WRL::ComPtr<ID3D11PixelShader>			pixelShader_;

	Microsoft::WRL::ComPtr<ID3D11InputLayout>			layout_;

	Microsoft::WRL::ComPtr<ID3D11Buffer>				constantBuffer_;

	Microsoft::WRL::ComPtr<ID3D11RasterizerState>		defaultRasteriserState_;
	Microsoft::WRL::ComPtr<ID3D11RasterizerState>		noCullRasteriserState_;

	Microsoft::WRL::ComPtr<ID3D11DepthStencilState>		stencilState_;

	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>	texture_;
};

