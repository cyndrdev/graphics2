#pragma once
#include <vector>

#include "Camera.h"
#include "DirectXCore.h"
#include "Framework.h"
#include "ResourceManager.h"
#include "GameConstants.h"
#include "SceneGraph.h"
#include "Lighting.h"

class DirectXFramework : public Framework
{
public:
	DirectXFramework();
	DirectXFramework(unsigned int width, unsigned int height);

	virtual void CreateSceneGraph();
	virtual void UpdateSceneGraph();

	bool Initialise();
	void Start();
	void Update();
	void Render();
	void OnResize(WPARAM wParam);
	void Shutdown();

	static DirectXFramework *				GetDXFramework();

	inline SceneGraphPointer				GetSceneGraph() { return sceneGraph_; }
	inline std::shared_ptr<Camera>			GetCamera() { return camera_; }
	inline std::shared_ptr<ResourceManager>	GetResourceManager() { return resourceManager_; }
	inline std::shared_ptr<Lighting>		GetLighting() { return lighting_; }
	inline ComPtr<ID3D11Device>				GetDevice() { return device_; }
	inline ComPtr<ID3D11DeviceContext>		GetDeviceContext() { return deviceContext_; }

	XMMATRIX								GetProjectionTransformation();

	void									SetBackgroundColour(XMFLOAT4 backgroundColour);

private:
	ComPtr<ID3D11Device>					device_;
	ComPtr<ID3D11DeviceContext>				deviceContext_;
	ComPtr<IDXGISwapChain>					swapChain_;
	ComPtr<ID3D11Texture2D>					depthStencilBuffer_;
	ComPtr<ID3D11RenderTargetView>			renderTargetView_;
	ComPtr<ID3D11DepthStencilView>			depthStencilView_;
	
	D3D11_VIEWPORT							screenViewport_;

	XMFLOAT4X4								projectionTransformation_;

	SceneGraphPointer						sceneGraph_;
	std::shared_ptr<ResourceManager>		resourceManager_;
	std::shared_ptr<Camera>					camera_;
	std::shared_ptr<Lighting>				lighting_;

	float									backgroundColour_[4];

	bool GetDeviceAndSwapChain();
};

