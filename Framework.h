#pragma once
#include "Core.h"
#include "Input.h"

class Framework
{
public:
	Framework();
	Framework(unsigned int width, unsigned int height);
	virtual ~Framework();

	int Run(HINSTANCE hInstance, int nCmdShow);

	LRESULT MsgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

	// getters
	inline unsigned int				GetWidth()		{ return width_; }
	inline unsigned int				GetHeight()		{ return height_; }
	inline HWND						GetHWnd()		{ return hWnd_; }
	inline std::shared_ptr<Input>	GetInput()		{ return input_;  }

	// virtual update methods
	virtual bool					Initialise()	{ return true; }
	virtual void					Start()			{ }
	virtual void					Update()		{ }
	virtual void					Render()		{ }
	virtual void					Shutdown()		{ }

	// window message handlers
	virtual void					OnKeyDown(WPARAM wParam)	{ }
	virtual void					OnKeyUp(WPARAM wParam)		{ }
	virtual void					OnResize(WPARAM wParam)		{ }

private:
	HINSTANCE						hInstance_;
	HWND							hWnd_;
	std::shared_ptr<Input>			input_;
	unsigned int					width_;
	unsigned int					height_;
	double							timeSpan_;

	bool							InitialiseMainWindow(int nCmdShow);
	int								MainLoop();
};
