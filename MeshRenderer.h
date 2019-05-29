#pragma once
#include "Renderer.h"
#include "Mesh.h"

class MeshRenderer : public Renderer
{
public:

	void SetMesh(std::shared_ptr<Mesh> mesh);
	void SetWorldTransformation(FXMMATRIX worldTransformation);
	void SetAmbientLight(XMFLOAT4 ambientLight);
	void SetDirectionalLight(FXMVECTOR lightVector, XMFLOAT4 lightColour);
	void SetCameraPosition(XMFLOAT4 cameraPosition);
	bool Initialise();
	void Render();
	void Shutdown(void);

private:
	std::shared_ptr<Mesh>			mesh_;
	XMFLOAT4X4						worldTransformation_;
	XMFLOAT4						ambientLight_;
	XMFLOAT4						directionalLightVector_;
	XMFLOAT4						directionalLightColour_;
	XMFLOAT4						cameraPosition_;

	ComPtr<ID3D11Device>			device_;
	ComPtr<ID3D11DeviceContext>		deviceContext_;

	ComPtr<ID3D11Buffer>			vertexBuffer_;
	ComPtr<ID3D11Buffer>			indexBuffer_;

	ComPtr<ID3DBlob>				vertexShaderByteCode_ = nullptr;
	ComPtr<ID3DBlob>				pixelShaderByteCode_ = nullptr;
	ComPtr<ID3D11VertexShader>		vertexShader_;
	ComPtr<ID3D11PixelShader>		pixelShader_;
	ComPtr<ID3D11InputLayout>		layout_;
	ComPtr<ID3D11Buffer>			constantBuffer_;

	ComPtr<ID3D11ShaderResourceView> texture_;

	ComPtr<ID3D11BlendState>		 transparentBlendState_;

	ComPtr<ID3D11RasterizerState>    defaultRasteriserState_;
	ComPtr<ID3D11RasterizerState>    noCullRasteriserState_;


	void BuildShaders();
	void BuildVertexLayout();
	void BuildConstantBuffer();
	void BuildBlendState();
	void BuildRendererState();

	void RenderNode(std::shared_ptr<Node> node, bool renderTransparent);
};

