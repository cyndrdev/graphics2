#include "Framework.h"
#include "GameConstants.h"

// reference to ourselves - primarily used to access the message handler correctly
Framework *	thisFramework_ = NULL;

// forward declaration of our window procedure
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

int APIENTRY wWinMain(
	_In_	   HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_	   LPWSTR    lpCmdLine,
	_In_	   int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	// we can only run if an instance of a class that inherits from Framework
	// has been created
	if (thisFramework_)
		return thisFramework_->Run(hInstance, nCmdShow);

	return -1;
}

Framework::Framework() : Framework(WINDOW_WIDTH, WINDOW_HEIGHT)
{
}

Framework::Framework(unsigned int width, unsigned int height)
{
	thisFramework_ = this;
	width_ = width;
	height_ = height;
}

Framework::~Framework()
{
}

int Framework::Run(HINSTANCE hInstance, int nCmdShow)
{
	int returnValue;

	hInstance_ = hInstance;
	if (!InitialiseMainWindow(nCmdShow))
	{
		return -1;
	}
	returnValue = MainLoop();
	Shutdown();
	return returnValue;
}

// Main program loop.  

int Framework::MainLoop()
{
	MSG msg;
	HACCEL hAccelTable = LoadAccelerators(hInstance_, MAKEINTRESOURCE(IDC_GRAPHICS2));
	LARGE_INTEGER counterFrequency;
	LARGE_INTEGER nextTime;
	LARGE_INTEGER currentTime;
	LARGE_INTEGER lastTime;
	bool updateFlag = true;

	// initialise timer
	QueryPerformanceFrequency(&counterFrequency);
	DWORD msPerFrame = (DWORD)(counterFrequency.QuadPart / WINDOW_FRAMERATE);
	double timeFactor = 1.0 / counterFrequency.QuadPart;
	QueryPerformanceCounter(&nextTime);
	lastTime = nextTime;

	// main message loop
	msg.message = WM_NULL;

	while (msg.message != WM_QUIT)
	{
		if (updateFlag)
		{
			QueryPerformanceCounter(&currentTime);
			timeSpan_ = (currentTime.QuadPart - lastTime.QuadPart) * timeFactor;
			lastTime = currentTime;
			Update();
			input_->Update();
			updateFlag = false;
		}

		QueryPerformanceCounter(&currentTime);

		// check if it's time for a new frame
		if (currentTime.QuadPart > nextTime.QuadPart)
		{
			Render();

			// set time for next frame
			nextTime.QuadPart += msPerFrame;

			// if we get more than a frame ahead, allow one to be dropped
			// otherwise, we will never catch up if we let the error accumulate

			if (nextTime.QuadPart < currentTime.QuadPart)
			{
				nextTime.QuadPart = currentTime.QuadPart + msPerFrame;
			}

			updateFlag = true;
		}
		else
		{
			if (PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
			{
				if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
				{
					TranslateMessage(&msg);
					DispatchMessage(&msg);
				}
			}
		}
	}
	return (int)msg.wParam;
}

// register the  window class, create the window and
// create the bitmap that we will use for rendering

bool Framework::InitialiseMainWindow(int nCmdShow)
{
	#define MAX_LOADSTRING 100

	WCHAR windowTitle[MAX_LOADSTRING];          
	WCHAR windowClass[MAX_LOADSTRING];            
	
	LoadStringW(hInstance_, IDS_APP_TITLE, windowTitle, MAX_LOADSTRING);
	LoadStringW(hInstance_, IDC_GRAPHICS2, windowClass, MAX_LOADSTRING);

	WNDCLASSEXW wcex;
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance_;
	wcex.hIcon = LoadIcon(hInstance_, MAKEINTRESOURCE(IDI_GRAPHICS2));
	wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = nullptr;
	wcex.lpszClassName = windowClass;
	wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	if (!RegisterClassExW(&wcex))
	{
		MessageBox(0, L"Unable to register window class", 0, 0);
		return false;
	}

	// now work out how large the window needs to be for our required client window size
	RECT windowRect = { 0, 0, static_cast<LONG>(width_), static_cast<LONG>(height_) };
	AdjustWindowRect(&windowRect, WS_OVERLAPPEDWINDOW, FALSE);
	width_	= windowRect.right - windowRect.left;
	height_ = windowRect.bottom - windowRect.top;

	hWnd_ = CreateWindowW(
		windowClass, 
		windowTitle, 
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		width_,
		height_,  
		nullptr,
		nullptr,
		hInstance_,
		nullptr
	);

	if (!hWnd_)
	{
		MessageBox(0, L"Unable to create window", 0, 0);
		return false;
	}

	if (!Initialise()) return false;

	input_ = std::make_shared<Input>(hWnd_);

	ShowWindow(hWnd_, nCmdShow);
	UpdateWindow(hWnd_);

	Start();
	return true;
}

// WndProc callback for current window

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	if (thisFramework_ != NULL)
	{
		// if framework is started, then we can call our own message proc
		return thisFramework_->MsgProc(hWnd, message, wParam, lParam);
	}
	else
	{
		// otherwise, we just pass control to the default message proc
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
}

// main WndProc

LRESULT Framework::MsgProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
		case WM_PAINT:
			break;

		case WM_KEYDOWN:
		case WM_SYSKEYDOWN:
			OnKeyDown(wParam);
			input_->ProcessKeyboard(message, wParam, lParam);
			return 0;

		case WM_KEYUP:
		case WM_SYSKEYUP:
			OnKeyUp(wParam);
			input_->ProcessKeyboard(message, wParam, lParam);
			return 0;

		case WM_INPUT:
		case WM_MOUSEMOVE:
		case WM_LBUTTONDOWN:
		case WM_LBUTTONUP:
		case WM_RBUTTONDOWN:
		case WM_RBUTTONUP:
		case WM_MBUTTONDOWN:
		case WM_MBUTTONUP:
		case WM_XBUTTONDOWN:
		case WM_XBUTTONUP:
		case WM_MOUSEHOVER:
		case WM_MOUSEWHEEL:
			input_->ProcessMouse(message, wParam, lParam);
			return 0;

		case WM_KILLFOCUS:
			input_->SetFocusState(false);
			return 0;

		case WM_SETFOCUS:
			input_->SetFocusState(true);
			return 0;

		case WM_DESTROY:
			PostQuitMessage(0);
			break;

		case WM_MOVE:
			Render();
			break;

		case WM_SIZE:
			width_	= LOWORD(lParam);
			height_	= HIWORD(lParam);
			OnResize(wParam);
			Render();
			break;

		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}
