#include "DirectXFramework.h"

// DirectX libraries that are needed
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d3dcompiler.lib")

DirectXFramework* dxFramework_ = nullptr;

DirectXFramework::DirectXFramework() : DirectXFramework(1280,720)
{
}

DirectXFramework::DirectXFramework(unsigned int width, unsigned int height) : Framework(width, height)
{
	dxFramework_ = this;

	// set our clear color
	backgroundColour_[0] = 0.1137f;
	backgroundColour_[1] = 0.0745f;
	backgroundColour_[2] = 0.2863f;
	backgroundColour_[3] = 0.0f;
}

DirectXFramework * DirectXFramework::GetDXFramework()
{
	return dxFramework_;
}

XMMATRIX DirectXFramework::GetProjectionTransformation()
{
	return XMLoadFloat4x4(&projectionTransformation_);
}

void DirectXFramework::SetBackgroundColour(XMFLOAT4 backgroundColour)
{
	backgroundColour_[0] = backgroundColour.x;
	backgroundColour_[1] = backgroundColour.y;
	backgroundColour_[2] = backgroundColour.z;
	backgroundColour_[3] = backgroundColour.w;
}

void DirectXFramework::CreateSceneGraph()
{
}

void DirectXFramework::UpdateSceneGraph()
{
}

bool DirectXFramework::Initialise()
{
	// required for WIC library
	HRESULT result = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);

	if FAILED(result) return false;
	if (!GetDeviceAndSwapChain()) return false;

	OnResize(SIZE_RESTORED);

	// create projection matrix
	XMStoreFloat4x4(&projectionTransformation_, XMMatrixPerspectiveFovLH(XM_PIDIV4, (float)GetWidth() / GetHeight(), 1.0f, 25000.0f));

	// initialise our pointers
	sceneGraph_			= std::make_shared<SceneGraph>();
	resourceManager_	= std::make_shared<ResourceManager>();
	camera_				= std::make_shared<Camera>();
	lighting_			= std::make_shared<Lighting>();

	CreateSceneGraph();
	return sceneGraph_->Initialise();
}

void DirectXFramework::Start()
{
	sceneGraph_->Start();
}

void DirectXFramework::Shutdown()
{
	sceneGraph_->Shutdown();
	CoUninitialize();
}

void DirectXFramework::Update()
{
	UpdateSceneGraph();
	camera_->Update();
	sceneGraph_->Update(XMMatrixIdentity());
}

void DirectXFramework::Render()
{
	// clear the render target and the depth stencil view
	deviceContext_->ClearRenderTargetView(renderTargetView_.Get(), backgroundColour_);
	deviceContext_->ClearDepthStencilView(depthStencilView_.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	// render the scene graph
	sceneGraph_->Render();

	// present the scene to the window
	HRESULT result = swapChain_->Present(0, 0);
	ThrowIfFailed(result);
}

void DirectXFramework::OnResize(WPARAM wParam)
{
	// update our projection matrix
	XMStoreFloat4x4(&projectionTransformation_, XMMatrixPerspectiveFovLH(XM_PIDIV4, (float)GetWidth() / GetHeight(), 1.0f, 10000.0f));

	// free depth views
	renderTargetView_ = nullptr;
	depthStencilView_ = nullptr;
	depthStencilBuffer_ = nullptr;

	ThrowIfFailed(swapChain_->ResizeBuffers(1, GetWidth(), GetHeight(), DXGI_FORMAT_R8G8B8A8_UNORM, 0));

	// create a new surface for rendering on
	ComPtr<ID3D11Texture2D> backBuffer;
	ThrowIfFailed(swapChain_->GetBuffer(0, IID_PPV_ARGS(&backBuffer)));
	ThrowIfFailed(device_->CreateRenderTargetView(backBuffer.Get(), NULL, renderTargetView_.GetAddressOf()));
	
	// === create our depth buffer === //
	D3D11_TEXTURE2D_DESC depthBufferTexture = { 0 };
	depthBufferTexture.Width = GetWidth();
	depthBufferTexture.Height = GetHeight();
	depthBufferTexture.ArraySize = 1;
	depthBufferTexture.MipLevels = 1;
	depthBufferTexture.SampleDesc.Count = 4;
	depthBufferTexture.Format = DXGI_FORMAT_D32_FLOAT;
	depthBufferTexture.Usage = D3D11_USAGE_DEFAULT;
	depthBufferTexture.BindFlags = D3D11_BIND_DEPTH_STENCIL;

	ComPtr<ID3D11Texture2D> depthBuffer;
	ThrowIfFailed(device_->CreateTexture2D(&depthBufferTexture, NULL, depthBuffer.GetAddressOf()));
	ThrowIfFailed(device_->CreateDepthStencilView(depthBuffer.Get(), 0, depthStencilView_.GetAddressOf()));

	deviceContext_->OMSetRenderTargets(1, renderTargetView_.GetAddressOf(), depthStencilView_.Get());

	// === create viewport === //
	D3D11_VIEWPORT viewPort;
	viewPort.Width = static_cast<float>(GetWidth());
	viewPort.Height = static_cast<float>(GetHeight());
	viewPort.MinDepth = 0.0f;
	viewPort.MaxDepth = 1.0f;
	viewPort.TopLeftX = 0;
	viewPort.TopLeftY = 0;
	deviceContext_->RSSetViewports(1, &viewPort);
}

bool DirectXFramework::GetDeviceAndSwapChain()
{
	UINT createDeviceFlags = 0;

	// We are going to only accept a hardware driver or a WARP
	// driver
	D3D_DRIVER_TYPE driverTypes[] =
	{
		D3D_DRIVER_TYPE_HARDWARE,
		D3D_DRIVER_TYPE_WARP
	};
	unsigned int totalDriverTypes = ARRAYSIZE(driverTypes);

	D3D_FEATURE_LEVEL featureLevels[] =
	{
		D3D_FEATURE_LEVEL_11_0
	};
	unsigned int totalFeatureLevels = ARRAYSIZE(featureLevels);

	DXGI_SWAP_CHAIN_DESC swapChainDesc = { 0 };
	swapChainDesc.BufferCount = 1;
	swapChainDesc.BufferDesc.Width = GetWidth();
	swapChainDesc.BufferDesc.Height = GetHeight();
	swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;

	// set the refresh rate to 0 and let DXGI figure out the optimal value
	swapChainDesc.BufferDesc.RefreshRate.Numerator = 0;
	swapChainDesc.BufferDesc.RefreshRate.Denominator = 0;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.OutputWindow = GetHWnd();

	// start out windowed
	swapChainDesc.Windowed = true;

	// turn on multisampling
	swapChainDesc.SampleDesc.Count = 4;
	swapChainDesc.SampleDesc.Quality = 0;

	// loop through the driver types to determine which one is available to us
	D3D_DRIVER_TYPE driverType = D3D_DRIVER_TYPE_UNKNOWN;
	HRESULT result;

	for (unsigned int driver = 0; driver < totalDriverTypes && driverType == D3D_DRIVER_TYPE_UNKNOWN; driver++)
	{
		result = D3D11CreateDeviceAndSwapChain(
			0,
			driverTypes[driver],
			0,
			createDeviceFlags,
			featureLevels,
			totalFeatureLevels,
			D3D11_SDK_VERSION,
			&swapChainDesc,
			swapChain_.GetAddressOf(),
			device_.GetAddressOf(),
			0,
			deviceContext_.GetAddressOf()
		);

		if (SUCCEEDED(result))
		{
			driverType = driverTypes[driver];
		}
	}
	if (driverType == D3D_DRIVER_TYPE_UNKNOWN)
	{
		// can't find a suitable driver
		return false;
	}
	return true;
}

