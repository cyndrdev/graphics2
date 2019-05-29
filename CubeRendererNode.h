#pragma once
#include "SceneNode.h"

class CubeRendererNode 
	: virtual public SceneNode
{
public:
	CubeRendererNode(std::wstring name);
	~CubeRendererNode();

	bool Initialise(void);
	void Start();
	void Render();
	void Update(FXMMATRIX& currentWorldTransformation);
	void Shutdown();
	void SetWorldTransform(FXMMATRIX& worldTransformation) { XMStoreFloat4x4(&worldTransformation_, worldTransformation); }
private:

	void BuildGeometryBuffers(void);
	void BuildShaders(void);
	void BuildVertexLayout(void);
	void BuildConstantBuffer(void);
	void BuildTexture(void);

	ComPtr<ID3D11Device>				_device;
	ComPtr<ID3D11DeviceContext>			_deviceContext;

	ComPtr<ID3D11Buffer>				_vertexBuffer;
	ComPtr<ID3D11Buffer>				_indexBuffer;

	ComPtr<ID3DBlob>					_vertexShaderByteCode = nullptr;
	ComPtr<ID3D11VertexShader>			_vertexShader;
	ComPtr<ID3DBlob>					_pixelShaderByteCode = nullptr;
	ComPtr<ID3D11PixelShader>			_pixelShader;

	ComPtr<ID3D11InputLayout>			_layout;

	ComPtr<ID3D11Buffer>				_constantBuffer;

	ComPtr<ID3D11ShaderResourceView>	_texture;
};

