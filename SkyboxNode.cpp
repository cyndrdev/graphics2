#include "SkyboxNode.h"
#include "DDSTextureLoader.h"
#include "DirectXFramework.h"
#include "DDSTextureLoader.h"
#include "WICTextureLoader.h"

struct CBUFFER {
	XMMATRIX CompleteTransformation;
};

SkyboxNode::SkyboxNode(std::wstring name, std::wstring boxTexturePath, float distance, int tessellation) :
	SceneNode(name),
	texturePath_(boxTexturePath),
	radius_(distance),
	tessellation_(tessellation)
{
}

SkyboxNode::~SkyboxNode()
{
}

bool SkyboxNode::Initialise(void)
{
	device_ = DirectXFramework::GetDXFramework()->GetDevice();
	deviceContext_ = DirectXFramework::GetDXFramework()->GetDeviceContext();

	CreateSphere(radius_, tessellation_);
	BuildShaders();
	BuildGeometryBuffers();
	BuildVertexLayout();
	BuildConstantBuffer();
	BuildTexture();
	BuildRendererStates();
	BuildDepthStencilState();

	return true;
}

void SkyboxNode::Start(void)
{
}

void SkyboxNode::Update(FXMMATRIX& currentWorldTransformation) {
	XMStoreFloat4x4(
		&combinedWorldTransformation_,
		XMMatrixTranslationFromVector(DirectXFramework::GetDXFramework()->GetCamera()->GetCameraPosition())
	);
}

void SkyboxNode::Render(void)
{
	XMFLOAT3 translation;
	XMStoreFloat3(
		&translation,
		DirectXFramework::GetDXFramework()->GetCamera()->GetCameraPosition()
	);

	XMMATRIX completeTransformation =
		XMMatrixTranslation(translation.x, translation.y, translation.z) *
		DirectXFramework::GetDXFramework()->GetCamera()->GetViewMatrix() *
		DirectXFramework::GetDXFramework()->GetProjectionTransformation();

	// create our constant buffer
	CBUFFER cBuffer;
	cBuffer.CompleteTransformation	= XMMatrixTranspose(completeTransformation);

	// set our buffer and shader resources
	deviceContext_->PSSetShader(pixelShader_.Get(), 0, 0);
	deviceContext_->VSSetShader(vertexShader_.Get(), 0, 0);

	deviceContext_->IASetInputLayout(layout_.Get());

	deviceContext_->UpdateSubresource(constantBuffer_.Get(), 0, 0, &cBuffer, 0, 0);
	deviceContext_->VSSetConstantBuffers(0, 1, constantBuffer_.GetAddressOf());
	deviceContext_->PSSetConstantBuffers(0, 1, constantBuffer_.GetAddressOf());

	deviceContext_->PSSetShaderResources(0, 1, texture_.GetAddressOf());

	UINT stride = sizeof(VERTEX);
	UINT offset = 0;

	deviceContext_->IASetVertexBuffers(0, 1, vertexBuffer_.GetAddressOf(), &stride, &offset);
	deviceContext_->IASetIndexBuffer(indexBuffer_.Get(), DXGI_FORMAT_R32_UINT, 0);
	deviceContext_->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	deviceContext_->RSSetState(noCullRasteriserState_.Get());
	deviceContext_->OMSetDepthStencilState(stencilState_.Get(), 1);
	deviceContext_->DrawIndexed(indexCount_, 0, 0);

	deviceContext_->OMSetDepthStencilState(nullptr, 1);
	deviceContext_->RSSetState(defaultRasteriserState_.Get());
}

void SkyboxNode::Shutdown(void)
{
}

void SkyboxNode::CreateSphere(float radius, size_t tessellation)
{
	size_t verticalSegments = tessellation;
	size_t horizontalSegments = tessellation * 2;
	size_t stride = horizontalSegments + 1;

	vertexCount_ = (verticalSegments+1) * (horizontalSegments+1);
	indexCount_ = (verticalSegments) * (horizontalSegments + 1) * 6;

	vertices_	= new VERTEX[vertexCount_];
	indices_	= new UINT[indexCount_];

	size_t vPos = 0;

	// create rings of vertices at progressively higher latitudes.
	for (size_t i = 0; i <= verticalSegments; i++)
	{
		float v = 1 - (float)i / verticalSegments;

		float latitude = (i * XM_PI / verticalSegments) - XM_PIDIV2;
		float dy, dxz;

		XMScalarSinCos(&dy, &dxz, latitude);

		// create a single ring of vertices at this latitude.
		for (size_t j = 0; j <= horizontalSegments; j++)
		{
			float u = (float)j / horizontalSegments;

			float longitude = j * XM_2PI / horizontalSegments;
			float dx, dz;

			XMScalarSinCos(&dx, &dz, longitude);

			dx *= dxz;
			dz *= dxz;

			VERTEX vertex;
			vertex.Position.x = dx * radius;
			vertex.Position.y = dy * radius;
			vertex.Position.z = dz * radius;
			vertices_[vPos++] = vertex;
		}
	}

	size_t iPos = 0;
	// fill the index buffer with triangles joining each pair of latitude rings.

	for (size_t i = 0; i < verticalSegments; i++)
	{
		for (size_t j = 0; j <= horizontalSegments; j++)
		{
			size_t nextI = i + 1;
			size_t nextJ = (j + 1) % stride;

			indices_[iPos++] = (UINT)(i * stride + j);
			indices_[iPos++] = (UINT)(nextI * stride + j);
			indices_[iPos++] = (UINT)(i * stride + nextJ);

			indices_[iPos++] = (UINT)(i * stride + nextJ);
			indices_[iPos++] = (UINT)(nextI * stride + j);
			indices_[iPos++] = (UINT)(nextI * stride + nextJ);
		}
	}
}

void SkyboxNode::BuildGeometryBuffers(void)
{
	/* === vertex buffer === */
	D3D11_BUFFER_DESC vertexBufferDesc;

	vertexBufferDesc.ByteWidth				= sizeof(VERTEX) * vertexCount_;
	vertexBufferDesc.Usage					= D3D11_USAGE_IMMUTABLE;
	vertexBufferDesc.BindFlags				= D3D11_BIND_VERTEX_BUFFER;
	vertexBufferDesc.CPUAccessFlags			= 0;
	vertexBufferDesc.MiscFlags				= 0;
	vertexBufferDesc.StructureByteStride	= 0;

	D3D11_SUBRESOURCE_DATA vertexInitData;
	vertexInitData.pSysMem = &vertices_[0];

	ThrowIfFailed(
		device_->CreateBuffer(
			&vertexBufferDesc,
			&vertexInitData,
			vertexBuffer_.GetAddressOf()
		)
	);

	/* === index buffer === */
	D3D11_BUFFER_DESC indexBufferDesc;

	indexBufferDesc.ByteWidth			= sizeof(UINT) * indexCount_;
	indexBufferDesc.Usage				= D3D11_USAGE_IMMUTABLE;
	indexBufferDesc.BindFlags			= D3D11_BIND_INDEX_BUFFER;
	indexBufferDesc.CPUAccessFlags		= 0;
	indexBufferDesc.MiscFlags			= 0;
	indexBufferDesc.StructureByteStride = 0;

	D3D11_SUBRESOURCE_DATA indexInitData;
	indexInitData.pSysMem = &indices_[0];

	ThrowIfFailed(
		device_->CreateBuffer(
			&indexBufferDesc,
			&indexInitData,
			indexBuffer_.GetAddressOf()
		)
	);
}

void SkyboxNode::BuildVertexLayout(void)
{
	D3D11_INPUT_ELEMENT_DESC vertexDesc[] = {
		{ "POSITION",	0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};

	ThrowIfFailed(
		device_->CreateInputLayout(
			vertexDesc,
			ARRAYSIZE(vertexDesc),
			vertexShaderByteCode_->GetBufferPointer(),
			vertexShaderByteCode_->GetBufferSize(),
			layout_.GetAddressOf()
		)
	);

	deviceContext_->IASetInputLayout(layout_.Get());
}

void SkyboxNode::BuildShaders(void)
{
	DWORD shaderCompileFlags = 0;
#if defined( _DEBUG )
	shaderCompileFlags |= D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

	ComPtr<ID3DBlob> compileLogs = nullptr;

	// compile our vertex shader
	HRESULT hr = D3DCompileFromFile(
		SKYBOX_SHADER.c_str(),
		nullptr,
		D3D_COMPILE_STANDARD_FILE_INCLUDE,
		"VS",
		"vs_5_0",
		shaderCompileFlags,
		0,
		vertexShaderByteCode_.GetAddressOf(),
		compileLogs.GetAddressOf()
	);

	// handle any errors
	if (compileLogs.Get() != nullptr)
		MessageBoxA(0, (char*)compileLogs->GetBufferPointer(), 0, 0);

	ThrowIfFailed(hr);

	// create & set the shader
	ThrowIfFailed(
		device_->CreateVertexShader(
			vertexShaderByteCode_->GetBufferPointer(),
			vertexShaderByteCode_->GetBufferSize(),
			NULL,
			vertexShader_.GetAddressOf()
		)
	);
	deviceContext_->VSSetShader(vertexShader_.Get(), 0, 0);

	// compile our pixel shader
	hr = D3DCompileFromFile(
		SKYBOX_SHADER.c_str(),
		nullptr,
		D3D_COMPILE_STANDARD_FILE_INCLUDE,
		"PS",
		"ps_5_0",
		shaderCompileFlags,
		0,
		pixelShaderByteCode_.GetAddressOf(),
		compileLogs.GetAddressOf()
	);

	// handle any errors
	if (compileLogs.Get() != nullptr)
		MessageBoxA(0, (char*)compileLogs->GetBufferPointer(), 0, 0);

	ThrowIfFailed(hr);

	// create & set the shader 
	ThrowIfFailed(
		device_->CreatePixelShader(
			pixelShaderByteCode_->GetBufferPointer(),
			pixelShaderByteCode_->GetBufferSize(),
			NULL,
			pixelShader_.GetAddressOf()
		)
	);
	deviceContext_->PSSetShader(pixelShader_.Get(), 0, 0);
}

void SkyboxNode::BuildConstantBuffer(void)
{
	D3D11_BUFFER_DESC bufferDesc;
	ZeroMemory(&bufferDesc, sizeof(bufferDesc));
	bufferDesc.Usage = D3D11_USAGE_DEFAULT;
	bufferDesc.ByteWidth = sizeof(CBUFFER);
	bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;

	ThrowIfFailed(
		device_->CreateBuffer(
			&bufferDesc, 
			NULL, 
			constantBuffer_.GetAddressOf()
		)
	);
}

void SkyboxNode::BuildTexture(void)
{
	ThrowIfFailed(
		CreateDDSTextureFromFile(
			device_.Get(),
			texturePath_.c_str(),
			nullptr,
			texture_.GetAddressOf()
		)
	);
}

void SkyboxNode::BuildRendererStates(void)
{
	// set default and nocull rasteriser states
	D3D11_RASTERIZER_DESC rasteriserDesc;
	rasteriserDesc.FillMode = D3D11_FILL_SOLID;
	rasteriserDesc.CullMode = D3D11_CULL_BACK;
	rasteriserDesc.FrontCounterClockwise = false;
	rasteriserDesc.DepthBias = 0;
	rasteriserDesc.SlopeScaledDepthBias = 0.0f;
	rasteriserDesc.DepthBiasClamp = 0.0f;
	rasteriserDesc.DepthClipEnable = true;
	rasteriserDesc.ScissorEnable = false;
	rasteriserDesc.MultisampleEnable = true;
	rasteriserDesc.AntialiasedLineEnable = true;
	ThrowIfFailed(device_->CreateRasterizerState(&rasteriserDesc, defaultRasteriserState_.GetAddressOf()));

	rasteriserDesc.CullMode = D3D11_CULL_NONE;
	//rasteriserDesc.FillMode = D3D11_FILL_WIREFRAME;
	ThrowIfFailed(device_->CreateRasterizerState(&rasteriserDesc, noCullRasteriserState_.GetAddressOf()));
}

void SkyboxNode::BuildDepthStencilState(void)
{
	D3D11_DEPTH_STENCIL_DESC stencilDesc;
	stencilDesc.DepthEnable = true;
	stencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	stencilDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
	stencilDesc.StencilEnable = false;
	stencilDesc.StencilReadMask = D3D11_DEFAULT_STENCIL_READ_MASK;
	stencilDesc.StencilWriteMask = D3D11_DEFAULT_STENCIL_WRITE_MASK;
	stencilDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
	stencilDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
	stencilDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	stencilDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	stencilDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
	stencilDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
	stencilDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	stencilDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	ThrowIfFailed(device_->CreateDepthStencilState(&stencilDesc, stencilState_.GetAddressOf()));
}
