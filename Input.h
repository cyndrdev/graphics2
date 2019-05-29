#pragma once
#include <Windows.h>
#include "DirectXCore.h"
#include <unordered_map>
#include <memory>

#define MOUSE_LEFT 0
#define MOUSE_MIDDLE 1
#define MOUSE_RIGHT 2
#define MOUSE_X1 3
#define MOUSE_X2 4

enum MouseButton {
	Left,
	Middle,
	Right,
	X1,
	X2
};

class Input
{
public:
	Input(HWND hWnd);
	~Input();
	void ProcessMouse(UINT message, WPARAM wParam, LPARAM lParam);
	void ProcessKeyboard(UINT message, WPARAM wParam, LPARAM lParam);
	void SetFocusState(bool state);

	bool IsKeyPressed(UINT key);
	bool IsKeyDown(UINT key);
	bool IsKeyUp(UINT key);

	bool IsButtonPressed(MouseButton button);
	bool IsButtonDown(MouseButton button);
	bool IsButtonUp(MouseButton button);

	XMFLOAT2 GetMouseDelta(void) { return mouseDelta_; }

	void Update(void);
private:
	// window handle
	HWND							hWnd_;

	// keyboard vars
	std::unordered_map<UINT, bool>	keyboardState_;
	std::unordered_map<UINT, bool>	keyboardStatePrev_;

	// mouse vars
	std::unordered_map<UINT, bool>	mouseState_;
	std::unordered_map<UINT, bool>	mouseStatePrev_;

	XMFLOAT2 mouseDelta_ { 0, 0 };
	bool isFocused_ = true;
};

