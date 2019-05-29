#include "MeshRenderer.h"
#include "DirectXFramework.h"

struct CBUFFER
{
	XMMATRIX    CompleteTransformation;
	XMMATRIX	WorldTransformation;
	XMFLOAT4	CameraPosition;
	XMVECTOR    LightVector;
	XMFLOAT4    LightColor;
	XMFLOAT4    AmbientColor;
	XMFLOAT4    DiffuseCoefficient;
	XMFLOAT4	SpecularCoefficient;
	float		Shininess;
	float		Opacity;
	float       Padding[2];
};

CBUFFER cBuffer;

void MeshRenderer::SetMesh(std::shared_ptr<Mesh> mesh)
{
	mesh_ = mesh;
}

void MeshRenderer::SetWorldTransformation(FXMMATRIX worldTransformation)
{
	XMStoreFloat4x4(&worldTransformation_, worldTransformation);
}

void MeshRenderer::SetAmbientLight(XMFLOAT4 ambientLight)
{
	ambientLight_ = ambientLight;
}

void MeshRenderer::SetDirectionalLight(FXMVECTOR lightVector, XMFLOAT4 lightColour)
{
	directionalLightColour_ = lightColour;
	XMStoreFloat4(&directionalLightVector_, lightVector);
}

void MeshRenderer::SetCameraPosition(XMFLOAT4 cameraPosition)
{
	cameraPosition_ = cameraPosition;
}

bool MeshRenderer::Initialise()
{
	device_ = DirectXFramework::GetDXFramework()->GetDevice();
	deviceContext_ = DirectXFramework::GetDXFramework()->GetDeviceContext();

	BuildShaders();
	BuildVertexLayout();
	BuildConstantBuffer();
	BuildBlendState();
	BuildRendererState();
	return true;
}

void MeshRenderer::RenderNode(std::shared_ptr<Node> node, bool renderTransparent)
{
	unsigned int subMeshCount = (unsigned int)node->GetMeshCount();

	// loop through all submeshes in the mesh, rendering them
	for (unsigned int i = 0; i < subMeshCount; i++)
	{
		unsigned int meshIndex = node->GetMesh(i);
		std::shared_ptr<SubMesh> subMesh = mesh_->GetSubMesh(meshIndex);
		std::shared_ptr<Material> material = subMesh->GetMaterial();
		float opacity = material->GetOpacity();
		if ((renderTransparent && opacity < 1.0f) ||
			(!renderTransparent && opacity == 1.0f))
		{
			UINT stride = sizeof(VERTEX);
			UINT offset = 0;

			vertexBuffer_ = subMesh->GetVertexBuffer();
			indexBuffer_ = subMesh->GetIndexBuffer();
			deviceContext_->IASetVertexBuffers(0, 1, vertexBuffer_.GetAddressOf(), &stride, &offset);
			deviceContext_->IASetIndexBuffer(indexBuffer_.Get(), DXGI_FORMAT_R32_UINT, 0);

			cBuffer.DiffuseCoefficient = material->GetDiffuseColour();
			cBuffer.SpecularCoefficient = material->GetSpecularColour();
			cBuffer.Shininess = material->GetShininess();
			cBuffer.Opacity = opacity;

			// update the constant buffer 
			deviceContext_->VSSetConstantBuffers(0, 1, constantBuffer_.GetAddressOf());
			deviceContext_->UpdateSubresource(constantBuffer_.Get(), 0, 0, &cBuffer, 0, 0);
			texture_ = material->GetTexture();
			deviceContext_->PSSetShaderResources(0, 1, texture_.GetAddressOf());
			deviceContext_->PSSetConstantBuffers(0, 1, constantBuffer_.GetAddressOf());
			deviceContext_->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
			deviceContext_->DrawIndexed(static_cast<UINT>(subMesh->GetIndexCount()), 0, 0);
		}
	}

	// render the children
	unsigned int childrenCount = (unsigned int)node->GetChildrenCount();

	// loop through all submeshes in the mesh, rendering them
	for (unsigned int i = 0; i < childrenCount; i++)
	{
		RenderNode(node->GetChild(i), renderTransparent);
	}
}

void MeshRenderer::Render()
{
	// turn off back face culling while we render a mesh. 
	// we do this since ASSIMP does not appear to be setting the
	// TWOSIDED property on materials correctly. without turning off
	// back face culling, some materials do not render correctly.
	deviceContext_->RSSetState(noCullRasteriserState_.Get());

	XMMATRIX projectionTransformation = DirectXFramework::GetDXFramework()->GetProjectionTransformation();
	XMMATRIX viewTransformation = DirectXFramework::GetDXFramework()->GetCamera()->GetViewMatrix();

	XMMATRIX completeTransformation = XMLoadFloat4x4(&worldTransformation_) * viewTransformation * projectionTransformation;

	cBuffer.CompleteTransformation = completeTransformation;
	cBuffer.WorldTransformation = XMLoadFloat4x4(&worldTransformation_);
	cBuffer.AmbientColor = ambientLight_;
	cBuffer.LightVector = XMVector4Normalize(XMLoadFloat4(&directionalLightVector_)); 
	cBuffer.LightColor = directionalLightColour_;
	cBuffer.CameraPosition = cameraPosition_;

	deviceContext_->VSSetShader(vertexShader_.Get(), 0, 0);
	deviceContext_->PSSetShader(pixelShader_.Get(), 0, 0);
	deviceContext_->IASetInputLayout(layout_.Get());

	// set the blend state correctly to handle opacity
	float blendFactors[] = { 0.0f, 0.0f, 0.0f, 0.0f }; 
	deviceContext_->OMSetBlendState(transparentBlendState_.Get(), blendFactors, 0xffffffff);

	// we do two passes through the nodes.  The first time we render nodes
	// that aren't transparent (i.e. their opacity == 1.0f).
	RenderNode(mesh_->GetRootNode(), false);

	// now we render any transparent nodes
	// we have to do this since blending always blends the submesh with
	// whatever is in the render target.  If we render a transparent node
	// first, it will be opaque.
	RenderNode(mesh_->GetRootNode(), true);

	// turn back face culling back on in case another renderer 
	// relies on it
	deviceContext_->RSSetState(defaultRasteriserState_.Get());
}

void MeshRenderer::Shutdown(void)
{
}

void MeshRenderer::BuildShaders()
{
	DWORD shaderCompileFlags = 0;
#if defined( _DEBUG )
	shaderCompileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

	ComPtr<ID3DBlob> compilationMessages = nullptr;

	// === vertex shader === //
	HRESULT hr = D3DCompileFromFile(
		MESH_SHADER.c_str(),
		nullptr, 
		D3D_COMPILE_STANDARD_FILE_INCLUDE,
		"VShader", 
		"vs_5_0",
		shaderCompileFlags, 
		0,
		vertexShaderByteCode_.GetAddressOf(),
		compilationMessages.GetAddressOf()
	);

	if (compilationMessages.Get() != nullptr)
	{
		// if there were any compilation messages, display them
		MessageBoxA(0, (char*)compilationMessages->GetBufferPointer(), 0, 0);
	}

	// even if there are no compiler messages, check to make sure there were no other errors.
	ThrowIfFailed(hr);
	ThrowIfFailed(
		device_->CreateVertexShader(
			vertexShaderByteCode_->GetBufferPointer(), 
			vertexShaderByteCode_->GetBufferSize(), 
			NULL, 
			vertexShader_.GetAddressOf()
		)
	);

	// === pixel shader === //
	hr = D3DCompileFromFile(
		MESH_SHADER.c_str(),
		nullptr,
		D3D_COMPILE_STANDARD_FILE_INCLUDE,
		"PShader",
		"ps_5_0",
		shaderCompileFlags,
		0,
		pixelShaderByteCode_.GetAddressOf(),
		compilationMessages.GetAddressOf()
	);

	if (compilationMessages.Get() != nullptr)
	{
		MessageBoxA(0, (char*)compilationMessages->GetBufferPointer(), 0, 0);
	}

	ThrowIfFailed(hr);
	ThrowIfFailed(
		device_->CreatePixelShader(
			pixelShaderByteCode_->GetBufferPointer(), 
			pixelShaderByteCode_->GetBufferSize(), 
			NULL, 
			pixelShader_.GetAddressOf()
		)
	);
}


void MeshRenderer::BuildVertexLayout()
{
	D3D11_INPUT_ELEMENT_DESC vertexDesc[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT , D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT , D3D11_INPUT_PER_VERTEX_DATA, 0 },
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
}

void MeshRenderer::BuildConstantBuffer()
{
	D3D11_BUFFER_DESC bufferDesc;
	ZeroMemory(&bufferDesc, sizeof(bufferDesc));
	bufferDesc.Usage = D3D11_USAGE_DEFAULT;
	bufferDesc.ByteWidth = sizeof(CBUFFER);
	bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	ThrowIfFailed(device_->CreateBuffer(&bufferDesc, NULL, constantBuffer_.GetAddressOf()));
}

void MeshRenderer::BuildBlendState()
{
	D3D11_BLEND_DESC transparentDesc = { 0 };
	transparentDesc.AlphaToCoverageEnable = false;
	transparentDesc.IndependentBlendEnable = false;
	transparentDesc.RenderTarget[0].BlendEnable = true;
	transparentDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
	transparentDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	transparentDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	transparentDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ZERO;
	transparentDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
	transparentDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	transparentDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
	ThrowIfFailed(device_->CreateBlendState(&transparentDesc, transparentBlendState_.GetAddressOf()));
}

void MeshRenderer::BuildRendererState()
{
	D3D11_RASTERIZER_DESC rasteriserDesc;
	rasteriserDesc.FillMode = D3D11_FILL_SOLID;
	rasteriserDesc.CullMode = D3D11_CULL_BACK;
	rasteriserDesc.FrontCounterClockwise = false;
	rasteriserDesc.DepthBias = 0;
	rasteriserDesc.SlopeScaledDepthBias = 0.0f;
	rasteriserDesc.DepthBiasClamp = 0.0f;
	rasteriserDesc.DepthClipEnable = true;
	rasteriserDesc.ScissorEnable = false;
	rasteriserDesc.MultisampleEnable = false;
	rasteriserDesc.AntialiasedLineEnable = false;
	ThrowIfFailed(device_->CreateRasterizerState(&rasteriserDesc, defaultRasteriserState_.GetAddressOf()));
	rasteriserDesc.CullMode = D3D11_CULL_NONE;
	ThrowIfFailed(device_->CreateRasterizerState(&rasteriserDesc, noCullRasteriserState_.GetAddressOf()));
}

