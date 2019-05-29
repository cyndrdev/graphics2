#include "TerrainNode.h"
#include "DirectXCore.h"
#include "DirectXFramework.h"
#include "DDSTextureLoader.h"
#include <algorithm>


struct CBUFFER {
	XMMATRIX	CompleteTransformation;
	XMMATRIX	WorldTransformation;
	XMVECTOR	CameraPosition;
	XMVECTOR	LightVector;
	XMFLOAT4	LightColor;
	XMFLOAT4	AmbientColor;
	XMFLOAT4	DiffuseCoefficient;
	XMFLOAT4	SpecularCoefficient;
	FLOAT		Shininess;
	FLOAT		Opacity;
	XMFLOAT2	Padding;
};

TerrainNode::TerrainNode(std::wstring name) :
SceneNode(name)
{
}

TerrainNode::~TerrainNode()
{
}

bool TerrainNode::Initialise()
{
	device_ = DirectXFramework::GetDXFramework()->GetDevice();
	deviceContext_ = DirectXFramework::GetDXFramework()->GetDeviceContext();

	if (!LoadHeightMap(HEIGHTMAP)) return false;

	LoadTerrainTextures();
	BuildShaders();

	BuildTerrainData();
	CalculateTerrainNormals();

	BuildGeometryBuffers();
	BuildVertexLayout();
	BuildConstantBuffer();
	BuildRendererStates();
	GenerateBlendMap();

	return true;
}

void TerrainNode::Start(void)
{
}

void TerrainNode::Render()
{
	XMMATRIX completeTransformation =
		XMLoadFloat4x4(&worldTransformation_) *
		DirectXFramework::GetDXFramework()->GetCamera()->GetViewMatrix() *
		DirectXFramework::GetDXFramework()->GetProjectionTransformation();

	// grab useful references
	std::shared_ptr<Camera> camera = DirectXFramework::GetDXFramework()->GetCamera();
	std::shared_ptr<Lighting> lighting = DirectXFramework::GetDXFramework()->GetLighting();

	// create our constant buffer
	CBUFFER cBuffer;
	cBuffer.CompleteTransformation	= completeTransformation;
	cBuffer.WorldTransformation		= XMLoadFloat4x4(&worldTransformation_);
	cBuffer.CameraPosition			= camera->GetCameraPosition();
	cBuffer.LightVector				= XMVector3Rotate(
		XMLoadFloat4(&lighting->DirectionalLightVector),
		XMQuaternionRotationRollPitchYaw(0, XM_PI, 0)
		);
	cBuffer.LightColor				= lighting->DirectionalLightColor;
	cBuffer.AmbientColor			= lighting->AmbientLight;
	cBuffer.DiffuseCoefficient		= XMFLOAT4(0.7f, 0.7f, 0.7f, 1.0f);
	cBuffer.SpecularCoefficient		= XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
	cBuffer.Shininess				= 0.0f;
	cBuffer.Opacity					= 1.0f;
	cBuffer.Padding					= XMFLOAT2(0.0f, 0.0f);

	// set our buffer and shader resources
	deviceContext_->PSSetShader(pixelShader_.Get(), 0, 0);
	deviceContext_->VSSetShader(vertexShader_.Get(), 0, 0);
	deviceContext_->IASetInputLayout(layout_.Get());

	deviceContext_->UpdateSubresource(constantBuffer_.Get(), 0, 0, &cBuffer, 0, 0);

	deviceContext_->VSSetConstantBuffers(0, 1, constantBuffer_.GetAddressOf());
	deviceContext_->PSSetConstantBuffers(0, 1, constantBuffer_.GetAddressOf());

	deviceContext_->PSSetShaderResources(0, 1, blendMapResourceView_.GetAddressOf());
	deviceContext_->PSSetShaderResources(1, 1, texturesResourceView_.GetAddressOf());

	// render our boi!
	UINT stride = sizeof(TERRAIN_VERTEX);
	UINT offset = 0;

	deviceContext_->IASetVertexBuffers(0, 1, vertexBuffer_.GetAddressOf(), &stride, &offset);
	deviceContext_->IASetIndexBuffer(indexBuffer_.Get(), DXGI_FORMAT_R32_UINT, 0);
	deviceContext_->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	deviceContext_->RSSetState(defaultRasteriserState_.Get());
	deviceContext_->DrawIndexed(INDEX_TARGET, 0, 0);
}

void TerrainNode::Update(FXMMATRIX& currentWorldTransformation)
{
}

void TerrainNode::Shutdown()
{
}

bool TerrainNode::LoadHeightMap(std::wstring filename)
{
	std::cout << "loading heightmap...\t\t";
	UINT mapSize = GRID_SIZE * GRID_SIZE;
	USHORT* rawFileValues = new USHORT[mapSize];

	std::ifstream inputHeightMap;
	inputHeightMap.open(filename.c_str(), std::ios_base::binary);
	if (!inputHeightMap) return false;

	inputHeightMap.read((char*)rawFileValues, mapSize * 2);
	inputHeightMap.close();

	for (UINT i = 0; i < mapSize; i++) {
		heightmap_.push_back((float)rawFileValues[i] / 65536);
	}

	delete[] rawFileValues;

	std::cout << "done!" << std::endl;
	return true;
}

void TerrainNode::LoadTerrainTextures(void)
{
	std::cout << "loading terrain textures...\t";

	ComPtr<ID3D11Resource> terrainTextures[5];

	for (int i = 0; i < 5; i++)
	{
		ThrowIfFailed(
			CreateDDSTextureFromFileEx(
				device_.Get(),
				deviceContext_.Get(),
				TERRAIN_TEXTURES[i].c_str(),
				0,
				D3D11_USAGE_IMMUTABLE,
				D3D11_BIND_SHADER_RESOURCE,
				0,
				0,
				false,
				terrainTextures[i].GetAddressOf(),
				nullptr
			)
		);
	}

	D3D11_TEXTURE2D_DESC textureDescription;
	ComPtr<ID3D11Texture2D> textureInterface;
	terrainTextures[0].As<ID3D11Texture2D>(&textureInterface);
	textureInterface->GetDesc(&textureDescription);

	D3D11_TEXTURE2D_DESC textureArrayDescription;
	textureArrayDescription.Width = textureDescription.Width;
	textureArrayDescription.Height = textureDescription.Height;
	textureArrayDescription.MipLevels = textureDescription.MipLevels;
	textureArrayDescription.ArraySize = 5;
	textureArrayDescription.Format = textureDescription.Format;
	textureArrayDescription.SampleDesc.Count = 1;
	textureArrayDescription.SampleDesc.Quality = 0;
	textureArrayDescription.Usage = D3D11_USAGE_DEFAULT;
	textureArrayDescription.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	textureArrayDescription.CPUAccessFlags = 0;
	textureArrayDescription.MiscFlags = 0;

	ComPtr<ID3D11Texture2D> textureArray = 0;
	ThrowIfFailed(
		device_->CreateTexture2D(
			&textureArrayDescription,
			0,
			textureArray.GetAddressOf()
		)
	);

	for (UINT i = 0; i < 5; i++)
	{
		// For each mipmap level...
		for (UINT mipLevel = 0; mipLevel < textureDescription.MipLevels; mipLevel++)
		{
			deviceContext_->CopySubresourceRegion(
				textureArray.Get(),
				D3D11CalcSubresource(mipLevel, i, textureDescription.MipLevels),
				NULL,
				NULL,
				NULL,
				terrainTextures[i].Get(),
				mipLevel,
				nullptr
			);
		}
	}

	// Create a resource view to the texture array.
	D3D11_SHADER_RESOURCE_VIEW_DESC viewDescription;
	viewDescription.Format = textureArrayDescription.Format;
	viewDescription.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DARRAY;
	viewDescription.Texture2DArray.MostDetailedMip = 0;
	viewDescription.Texture2DArray.MipLevels = textureArrayDescription.MipLevels;
	viewDescription.Texture2DArray.FirstArraySlice = 0;
	viewDescription.Texture2DArray.ArraySize = 5;

	ThrowIfFailed(
		device_->CreateShaderResourceView(
			textureArray.Get(),
			&viewDescription,
			texturesResourceView_.GetAddressOf()
		)
	);

	std::cout << "done." << std::endl;
}

void TerrainNode::GenerateBlendMap(void)
{
	std::cout << "generating blend map...\t\t";
	DWORD* blendMap = new DWORD[(GRID_SIZE-1) * (GRID_SIZE-1)];
	DWORD* blendMapPtr = blendMap;
	BYTE r = 0;
	BYTE g = 0;
	BYTE b = 0;
	BYTE a = 0;

	DWORD index = 0;
	for (DWORD i = 0; i < GRID_SIZE-1; i++)
	{
		for (DWORD j = 0; j < GRID_SIZE-1; j++)
		{
			//r = 255 * sin(float(i) * AI_MATH_HALF_PI_F / GRID_SIZE);
			//g = 255 * sin(float(j) * AI_MATH_HALF_PI_F / GRID_SIZE);
			//b = 255 * abs(sin(float(i) * 3.0f * AI_MATH_HALF_PI_F / GRID_SIZE));
			//a = 255 * abs(sin(float(j) * 3.0f * AI_MATH_HALF_PI_F / GRID_SIZE));
			float height = vertices_[((j * (GRID_SIZE - 1)) + i) * 4].Position.y;

			r = 0;
			g = 0;
			b = 0;
			a = 0;

			b = (BYTE)std::clamp(height/ 2.0f, 0.0f, 255.0f);
			a = (BYTE)std::clamp((height/ 2.0f), 200.0f, 455.0f) - 200.0f;

			DWORD mapValue = (a << 24) + (b << 16) + (g << 8) + r;
			*blendMapPtr++ = mapValue;
		}
	}
	D3D11_TEXTURE2D_DESC blendMapDescription;
	blendMapDescription.Width = GRID_SIZE-1;
	blendMapDescription.Height = GRID_SIZE-1;
	blendMapDescription.MipLevels = 1;
	blendMapDescription.ArraySize = 1;
	blendMapDescription.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	blendMapDescription.SampleDesc.Count = 1;
	blendMapDescription.SampleDesc.Quality = 0;
	blendMapDescription.Usage = D3D11_USAGE_DEFAULT;
	blendMapDescription.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	blendMapDescription.CPUAccessFlags = 0;
	blendMapDescription.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA blendMapInitialisationData;
	blendMapInitialisationData.pSysMem = blendMap;
	blendMapInitialisationData.SysMemPitch = 4 * (GRID_SIZE - 1);

	ComPtr<ID3D11Texture2D> blendMapTexture;
	ThrowIfFailed(
		device_->CreateTexture2D(
			&blendMapDescription,
			&blendMapInitialisationData,
			blendMapTexture.GetAddressOf()
		)
	);

	// Create a resource view to the texture array.
	D3D11_SHADER_RESOURCE_VIEW_DESC viewDescription;
	viewDescription.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	viewDescription.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	viewDescription.Texture2D.MostDetailedMip = 0;
	viewDescription.Texture2D.MipLevels = 1;

	ThrowIfFailed(
		device_->CreateShaderResourceView(
			blendMapTexture.Get(),
			&viewDescription,
			blendMapResourceView_.GetAddressOf()
		)
	);

	delete[] blendMap;

	std::cout << "done." << std::endl << std::endl;
}

void TerrainNode::BuildTerrainData(void)
{
	std::cout << std::endl;
	std::cout << ":: generating terrain" << std::endl;

	std::cout << "allocating memory...\t\t";

	std::cout << "done!" << std::endl;

	std::cout << "generating geometry:\t\t";
	StartStatbar();

	int step = 0;
	size_t vPos = 0;
	size_t iPos = 0;
	int half = GRID_SIZE / 2;

	TERRAIN_VERTEX vertex;
	vertex.Normal = { 0, 0, 0 };

	int nextValue = (GRID_SIZE-1) / 100;
	for (int x = 0; x < GRID_SIZE - 1; x++) {
		for (int z = 0; z < GRID_SIZE - 1; z++) {
			// indices
			indices_[iPos++] = vPos;
			indices_[iPos++] = vPos+1;
			indices_[iPos++] = vPos+2;
			indices_[iPos++] = vPos+2;
			indices_[iPos++] = vPos+1;
			indices_[iPos++] = vPos+3;

			// vertices
			vertex.BlendMapTexCoord = {
				(float)x / (GRID_SIZE - 1),
				(float)z / (GRID_SIZE - 1),
			};

			// v1
			vertex.TexCoord = { 0, 0 };
	
			vertex.Position = {
				(x - half) * GRID_STEP,
				heightmap_[(x * GRID_SIZE) + z] * GRID_MAGNITUDE,
				(z - half) * GRID_STEP
			};

			vertices_[vPos++] = vertex;

			// v2
			vertex.TexCoord = { 0, 1 };

			vertex.Position = {
				(x - half) * GRID_STEP,
				heightmap_[(x * GRID_SIZE) + (z+1)] * GRID_MAGNITUDE,
				(z+1 - half) * GRID_STEP
			};

			vertices_[vPos++] = vertex;

			// v3
			vertex.TexCoord = { 1, 0 };
	
			vertex.Position = {
				(x+1 - half) * GRID_STEP,
				heightmap_[((x+1) * GRID_SIZE) + z] * GRID_MAGNITUDE,
				(z - half) * GRID_STEP
			};

			vertices_[vPos++] = vertex;

			// v4
			vertex.TexCoord = { 1, 1 };
	
			vertex.Position = {
				(x+1 - half) * GRID_STEP,
				heightmap_[((x+1) * GRID_SIZE) + (z+1)] * GRID_MAGNITUDE,
				(z+1 - half) * GRID_STEP
			};

			vertices_[vPos++] = vertex;
		}
		if (x == nextValue) {
			UpdateStatbar(++step);
			nextValue = ((step + 1) * GRID_SIZE - 1) / 100;
		}
	}

	UpdateStatbar(100);
}

void TerrainNode::CalculateTerrainNormals(void)
{
	std::cout << "calculating normals:\t\t";
	StartStatbar();

	int step = 0;
	int nextValue = (GRID_SIZE-1) / 100;
	for (size_t pos = 0; pos < VERTEX_TARGET; pos += 4) {
		XMVECTOR vec1 = XMLoadFloat3(&vertices_[pos + 0].Position);
		XMVECTOR vec2 = XMLoadFloat3(&vertices_[pos + 1].Position);
		XMVECTOR vec3 = XMLoadFloat3(&vertices_[pos + 2].Position);
		XMVECTOR vec4 = XMLoadFloat3(&vertices_[pos + 3].Position);

		XMVECTOR normalOne = XMVector3Cross(vec2 - vec1, vec3 - vec1);
		XMVECTOR normalTwo = XMVector3Cross(vec3 - vec2, vec3 - vec4);

		XMStoreFloat3(&vertices_[pos + 0].Normal, normalOne);
		XMStoreFloat3(&vertices_[pos + 1].Normal, normalOne + normalTwo);
		XMStoreFloat3(&vertices_[pos + 2].Normal, normalOne + normalTwo);
		XMStoreFloat3(&vertices_[pos + 3].Normal, normalTwo);

		if (pos >= nextValue) {
			nextValue += VERTEX_TARGET / 100;
			UpdateStatbar(++step);
		}
	}

	for (size_t pos = 0; pos < VERTEX_TARGET; pos++) {
		XMVECTOR normal = XMVector3Normalize(XMLoadFloat3(&vertices_[pos].Normal));
		XMStoreFloat3(&vertices_[pos].Normal, normal);
	}

	UpdateStatbar(100);
}

void TerrainNode::BuildGeometryBuffers(void)
{
	/* === vertex buffer === */
	D3D11_BUFFER_DESC vertexBufferDesc;

	vertexBufferDesc.ByteWidth				= sizeof(TERRAIN_VERTEX) * VERTEX_TARGET;
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

	indexBufferDesc.ByteWidth			= sizeof(UINT) * INDEX_TARGET;
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

void TerrainNode::BuildVertexLayout(void)
{
	D3D11_INPUT_ELEMENT_DESC vertexDesc[] = {
		{ "POSITION",	0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,								D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL",		0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT,	D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD",	0, DXGI_FORMAT_R32G32_FLOAT,	0, D3D11_APPEND_ALIGNED_ELEMENT,	D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD",	1, DXGI_FORMAT_R32G32_FLOAT,	0, D3D11_APPEND_ALIGNED_ELEMENT,	D3D11_INPUT_PER_VERTEX_DATA, 0 },
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

void TerrainNode::BuildShaders(void)
{
	DWORD shaderCompileFlags = 0;
#if defined( _DEBUG )
	shaderCompileFlags |= D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

	ComPtr<ID3DBlob> compileLogs = nullptr;

	// compile our vertex shader
	HRESULT hr = D3DCompileFromFile(
		TERRAIN_SHADER.c_str(),
		nullptr,
		D3D_COMPILE_STANDARD_FILE_INCLUDE,
		"VShader",
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
		TERRAIN_SHADER.c_str(),
		nullptr,
		D3D_COMPILE_STANDARD_FILE_INCLUDE,
		"PShader",
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

void TerrainNode::BuildConstantBuffer(void)
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

void TerrainNode::BuildRendererStates(void)
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
	rasteriserDesc.MultisampleEnable = true;
	rasteriserDesc.AntialiasedLineEnable = true;
	ThrowIfFailed(device_->CreateRasterizerState(&rasteriserDesc, defaultRasteriserState_.GetAddressOf()));
	rasteriserDesc.MultisampleEnable = false;
	rasteriserDesc.AntialiasedLineEnable = false;
	rasteriserDesc.FillMode = D3D11_FILL_WIREFRAME;
	ThrowIfFailed(device_->CreateRasterizerState(&rasteriserDesc, wireframeRasteriserState_.GetAddressOf()));

}

float TerrainNode::GetHeightAtPoint(float x, float z)
{
	bool snapped = false;
	int half = GRID_SIZE / 2;

	float cellXin = (x / GRID_STEP) + half;
	float cellZin = (z / GRID_STEP) + half;

	int cellX = (int)floor(cellXin);
	int cellZ = (int)floor(cellZin);

	float dx = fmodf(x, GRID_STEP);
	float dz = fmodf(z, GRID_STEP);

	// get rid of negative values at negative coords
	if (dx < 0.0f) dx += GRID_STEP;
	if (dz < 0.0f) dz += GRID_STEP;

	// clamp sides
	if (cellX < 0) {
		cellX = 0;
		dx = 0.0f;
	}

	if (cellZ < 0) {
		cellZ = 0;
		dz = 0.0f;
	}

	if (cellX >= GRID_SIZE - 1) {
		cellX = GRID_SIZE - 2;
		dx = GRID_STEP;
	}

	if (cellZ >= GRID_SIZE - 1) {
		cellZ = GRID_SIZE - 2;
		dz = GRID_STEP;
	}

	// check which tri we're within
	bool secondTri = (dx + dz > GRID_STEP);

	int terrainIndex = ((cellX * (GRID_SIZE - 1)) + cellZ) * 4;

	TERRAIN_VERTEX* baseVertex = &vertices_[terrainIndex + (secondTri ? 3 : 0)];
	
	if (secondTri) {
		// we're counting backwards so our differences should be backwards
		dx -= GRID_STEP;
		dz -= GRID_STEP;
	}

	XMFLOAT3 normal = baseVertex->Normal;
	XMFLOAT3 position = baseVertex->Position;

	return position.y + ((normal.x * dx + normal.z * dz) / -normal.y);
}
