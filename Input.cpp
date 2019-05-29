#include "Input.h"
#include "DirectXCore.h"
#include <iostream>

Input::Input(HWND hWnd)
{
	hWnd_				= hWnd;
	keyboardState_		= std::unordered_map<UINT, bool>();
	keyboardStatePrev_	= std::unordered_map<UINT, bool>();

	mouseState_			= std::unordered_map<UINT, bool>();
	mouseStatePrev_		= std::unordered_map<UINT, bool>();
}


Input::~Input()
{
}

void Input::ProcessMouse(UINT message, WPARAM wParam, LPARAM lParam)
{
	int buttonId;
	switch (message) {
	case WM_INPUT:
	case WM_MOUSEMOVE:
	case WM_MOUSEHOVER:
	case WM_MOUSEWHEEL:
		// we don't want to deal with these
		return;

	case WM_LBUTTONDOWN:
	case WM_RBUTTONDOWN:
	case WM_MBUTTONDOWN:
	case WM_XBUTTONDOWN:
	case WM_LBUTTONUP:
	case WM_RBUTTONUP:
	case WM_MBUTTONUP:
	case WM_XBUTTONUP:
		// update state according to wParam's flags
		// https://docs.microsoft.com/en-us/windows/desktop/learnwin32/mouse-clicks
		mouseState_[MouseButton::Left]		= MK_LBUTTON	& wParam;
		mouseState_[MouseButton::Middle]	= MK_MBUTTON	& wParam;
		mouseState_[MouseButton::Right]		= MK_RBUTTON	& wParam;
		mouseState_[MouseButton::X1]		= MK_XBUTTON1	& wParam;
		mouseState_[MouseButton::X2]		= MK_XBUTTON2	& wParam;
	}
}

void Input::ProcessKeyboard(UINT message, WPARAM wParam, LPARAM lParam)
{
	WCHAR name[256];
	int result = GetKeyNameTextW(lParam, name, 256);
	switch (message) {
	case WM_KEYDOWN:
	case WM_SYSKEYDOWN:
		if (keyboardState_[wParam]) return;
		keyboardState_[wParam] = true;
		std::cout << "+";
		break;

	case WM_KEYUP:
	case WM_SYSKEYUP:
		if (!keyboardState_[wParam]) return;
		keyboardState_[wParam] = false;
		std::cout << "-";
		break;
	}

	std::wcout << name << std::endl;
}

bool Input::IsKeyPressed(UINT key)
{
	auto value = keyboardState_.find(key);
	return (value == keyboardState_.end())
		? false
		: value->second;
}

bool Input::IsKeyDown(UINT key)
{
	auto newValue = keyboardState_.find(key);
	auto oldValue = keyboardStatePrev_.find(key);
	return (newValue == keyboardState_.end())
		? false
		: newValue->second && (oldValue == keyboardStatePrev_.end() || !oldValue->second);
}

bool Input::IsKeyUp(UINT key)
{
	auto newValue = keyboardState_.find(key);
	auto oldValue = keyboardStatePrev_.find(key);
	return (oldValue == keyboardStatePrev_.end() || newValue == keyboardState_.end())
		? false
		: !newValue->second && oldValue->second;
}

bool Input::IsButtonPressed(MouseButton button)
{
	auto value = mouseState_.find(button);
	return (value == mouseState_.end())
		? false
		: value->second;
}

bool Input::IsButtonDown(MouseButton button)
{
	auto newValue = mouseState_.find(button);
	auto oldValue = mouseStatePrev_.find(button);
	return (newValue == mouseState_.end())
		? false
		: newValue->second && (oldValue == mouseStatePrev_.end() || !oldValue->second);
}

bool Input::IsButtonUp(MouseButton button)
{
	auto newValue = mouseState_.find(button);
	auto oldValue = mouseStatePrev_.find(button);
	return (oldValue == mouseStatePrev_.end() || newValue == mouseState_.end())
		? false
		: !newValue->second && oldValue->second;
}

void Input::Update()
{
	if (IsKeyDown(VK_ESCAPE))				SetFocusState(false);
	if (IsButtonDown(MouseButton::Left))	SetFocusState(true);

	if (isFocused_) {
		RECT wRect;
		POINT cPos;

		GetCursorPos(&cPos);
		GetWindowRect(hWnd_, &wRect);

		LONG midX = (wRect.left + wRect.right) / 2;
		LONG midY = (wRect.top + wRect.bottom) / 2;

		mouseDelta_ = { (float)(midX - cPos.x), (float)(midY - cPos.y) };
		SetCursorPos(midX, midY);
	}
	else {
		mouseDelta_ = { 0, 0 };
	}

	keyboardStatePrev_ = keyboardState_;
	mouseStatePrev_ = mouseState_;
}

void Input::SetFocusState(bool state)
{
	isFocused_ = state;
	// this might seem to make no sense - I swear it does though!
	// https://docs.microsoft.com/en-us/windows/desktop/api/winuser/nf-winuser-showcursor
	while (state && ShowCursor(false) >= 0);
	while (!state && ShowCursor(true) < 0);

	if (!state) {
		// we're setting the focus off, reset all our values
		for (auto& pair : keyboardState_) {
			pair.second = false;
		}
	}

	std::cout << (state ? "got" : "lost") << " focus!" << std::endl;
}
