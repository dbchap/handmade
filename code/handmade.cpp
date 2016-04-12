// day 006 - Gamepad and Keyboard Input
#include <windows.h>
#include <stdint.h>
#include <math.h>
#include <xinput.h>

#define internal static
#define local_persist static 
#define global_variable static

typedef uint8_t uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;

typedef int8_t int8;
typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;

global_variable bool running;


struct win32_offscreen_buffer {
	BITMAPINFO info;
	void *memory;
	int width;
	int height;
	int pitch;
	int bytesPerPixel = 4;
};

struct win32_window_dimensions {
	int width;
	int height;
};

global_variable win32_offscreen_buffer globalBackBuffer;

// XInputGetState wrapper to prevent null pointer
#define X_INPUT_GET_STATE(name) DWORD WINAPI name(DWORD dwUserIndex, XINPUT_STATE* pState)
typedef X_INPUT_GET_STATE(x_input_get_state);
X_INPUT_GET_STATE(XInputGetStateStub) {
	return(0);
}
global_variable x_input_get_state *XInputGetState_ = XInputGetStateStub;

// XInputSetState wrapper to prevent null pointer
#define X_INPUT_SET_STATE(name) DWORD WINAPI name(DWORD dwUserIndex, XINPUT_VIBRATION* pVibration)
typedef X_INPUT_SET_STATE(x_input_set_state);
X_INPUT_SET_STATE(XInputSetStateStub) {
	return(0);
}
global_variable x_input_set_state *XInputSetState_ = XInputSetStateStub;

typedef DWORD WINAPI x_input_get_state(DWORD dwUserIndex, XINPUT_STATE* pState);
typedef DWORD WINAPI x_input_set_state(DWORD dwUserIndex, XINPUT_VIBRATION* pVibration);

#define XInputGetState XInputGetState_
#define XInputSetState XInputSetState_


internal void loadXInput(void) {
	HMODULE XInputLibrary = LoadLibrary("xinput1_3.dll");
	if ( XInputLibrary ) {
		XInputGetState = (x_input_get_state*)GetProcAddress(XInputLibrary, "XInputGetState");
		XInputSetState = (x_input_set_state*)GetProcAddress(XInputLibrary, "XInputSetState");
	}
}

win32_window_dimensions getWindowDimensions(HWND window) {

	RECT clientRect;
	GetClientRect(window, &clientRect);
	int windowWidth = clientRect.right - clientRect.left;
	int windowHeight = clientRect.bottom - clientRect.top;
	win32_window_dimensions dimensions = {windowWidth, windowHeight};
	return dimensions;

}

internal void renderWeirdGradient(win32_offscreen_buffer buffer, int xOffset, int  yOffset)  {

	uint8 *row = (uint8*)buffer.memory;
	for ( int y = 0; y < buffer.height; ++y ) {
		uint32 *pixel = (uint32*)row;
		for ( int x = 0; x < buffer.width; ++x ) {
			/*

			Pixel in memory: BB GG RR xx
			*/
			uint8 r = 0;
			uint8 g = (y + yOffset);
			uint8 b = (x + xOffset);
			uint8 padd = 0;
			*pixel++ = (uint32)(padd<<24|r<<16|g<<8|b);

			/**
			Memory:   BB GG RR xx
			Register: xx RR GG BB
			0000 0000 0000 0000
			Pixel (32-bits)
			*/

		}
		row += buffer.pitch;
	}
}



// Device independent bitmap
internal void
resizeDIBSection(win32_offscreen_buffer *buffer, int width, int height) {


	// TODO: bulletproof this
	// maybe don't free first, free after, then free first if that fails.
	
	if ( buffer->memory ) {
		VirtualFree(buffer->memory, 0, MEM_RELEASE);
	}

	buffer->width = width;
	buffer->height = height;

	buffer->info.bmiHeader.biSize = sizeof(buffer->info.bmiHeader);
	buffer->info.bmiHeader.biWidth = buffer->width;
	buffer->info.bmiHeader.biHeight = -buffer->height; // negative for top-down co-ordinates
	buffer->info.bmiHeader.biPlanes = 1;
	buffer->info.bmiHeader.biBitCount = 32;
	buffer->info.bmiHeader.biCompression = BI_RGB;

	int bitmapMemorySize = (buffer->width*buffer->height)*buffer->bytesPerPixel;
	buffer->memory = VirtualAlloc(0, bitmapMemorySize, MEM_COMMIT, PAGE_READWRITE);

	buffer->pitch = width*buffer->bytesPerPixel;

	// TODO: clear this to black.
}

internal void
displayBufferInWindow(win32_offscreen_buffer *buffer, HDC deviceContext, int windowWidth, int windowHeight) {

	//TODO: aspect ratio correction.

	StretchDIBits(deviceContext, /*
					x, y, width, height,
					x, y, width, height,*/
					0,0, windowWidth, windowHeight,
					0,0, buffer->width, buffer->height,
					buffer->memory,
					&buffer->info,
					DIB_RGB_COLORS, // rgb buffer, or palatized
					SRCCOPY);
}


LRESULT CALLBACK 
MainWindowProc( HWND window, UINT message, WPARAM wParam, LPARAM lParam ) {

	LRESULT result = 0;

	switch ( message ) {
		case WM_SIZE: {
			}break;
		case WM_CLOSE: {
			running = false;
		}break;
		case WM_DESTROY: {
			running = false;
			}break;
		
		case WM_ACTIVATEAPP: {
			OutputDebugStringA("WM_ACTIVATEAPP\n");
		} break;
		case WM_PAINT:
		{
			PAINTSTRUCT paint;
			HDC deviceContext = BeginPaint(window,&paint);

			win32_window_dimensions windowDimensions = getWindowDimensions(window);

			displayBufferInWindow(&globalBackBuffer, deviceContext, windowDimensions.width,windowDimensions.height);
			EndPaint(window, &paint);
		}	break;

		default: {
			OutputDebugStringA("default\n");
			result = DefWindowProc(window, message, wParam, lParam);
			}break;
	}

	return result;
}

int CALLBACK WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, 
	LPSTR lpCmdLine, int nCmdShow){

	loadXInput();

	WNDCLASS windowClass = {};

	resizeDIBSection(&globalBackBuffer, 1280, 720);

	// TODO: Check if HREDRAW/VREDRAW/OWNDC still matter.
	windowClass.style = CS_HREDRAW|CS_VREDRAW;
	windowClass.lpfnWndProc = MainWindowProc;
	windowClass.hInstance = hInstance;
	// WindowClass.hIcon;
	windowClass.lpszClassName = "HandMadeHeroWindowClass";

	if ( RegisterClass(&windowClass) ) {
		HWND windowHandle = CreateWindowEx(
				0,
				windowClass.lpszClassName,
				"Handmade Hero",
				WS_OVERLAPPEDWINDOW|WS_VISIBLE,
				CW_USEDEFAULT,
				CW_USEDEFAULT,
				CW_USEDEFAULT,
				CW_USEDEFAULT,
				0,
				0,
				hInstance,
				0);
		if ( windowHandle ) {
			
			running = true;

			int xOffset = 0;
			int yOffset = 0;

			while ( running ) {

				MSG message;
				while ( PeekMessage(&message, 0, 0, 0, PM_REMOVE) ) {

					if ( message.message == WM_QUIT ) {
						running = false;
					}

					TranslateMessage(&message);
					DispatchMessage(&message);
				}

				// should we poll this more frequently?
				for ( DWORD controllerIdx = 0; controllerIdx < XUSER_MAX_COUNT; ++controllerIdx ) {
					XINPUT_STATE controllerState;
					
					// this controller is plugged in
					if ( XInputGetState(controllerIdx, &controllerState) == ERROR_SUCCESS ) {

						/*
						_XINPUT_GAMEPAD {
						  WORD  wButtons;
						  BYTE  bLeftTrigger;
						  BYTE  bRightTrigger;
						  SHORT sThumbLX;
						  SHORT sThumbLY;
						  SHORT sThumbRX;
						  SHORT sThumbRY;
						} XINPUT_GAMEPAD, *PXINPUT_GAMEPAD;

						XINPUT_GAMEPAD_DPAD_UP	0x0001
						XINPUT_GAMEPAD_DPAD_DOWN	0x0002
						XINPUT_GAMEPAD_DPAD_LEFT	0x0004
						XINPUT_GAMEPAD_DPAD_RIGHT	0x0008
						XINPUT_GAMEPAD_START	0x0010
						XINPUT_GAMEPAD_BACK	0x0020
						XINPUT_GAMEPAD_LEFT_THUMB	0x0040
						XINPUT_GAMEPAD_RIGHT_THUMB	0x0080
						XINPUT_GAMEPAD_LEFT_SHOULDER	0x0100
						XINPUT_GAMEPAD_RIGHT_SHOULDER	0x0200
						XINPUT_GAMEPAD_A	0x1000
						XINPUT_GAMEPAD_B	0x2000
						XINPUT_GAMEPAD_X	0x4000
						XINPUT_GAMEPAD_Y	0x8000*/

						//  see if controllerstate.dwPacketNumber increments too quickly
						XINPUT_GAMEPAD *pad = &controllerState.Gamepad;
						bool up = (pad->wButtons & XINPUT_GAMEPAD_DPAD_UP);
						bool down = (pad->wButtons & XINPUT_GAMEPAD_DPAD_DOWN);
						bool left = (pad->wButtons & XINPUT_GAMEPAD_DPAD_LEFT);
						bool right = (pad->wButtons & XINPUT_GAMEPAD_DPAD_RIGHT);
						bool start = (pad->wButtons & XINPUT_GAMEPAD_START);
						bool back = (pad->wButtons & XINPUT_GAMEPAD_BACK);
						bool leftThumb = (pad->wButtons & XINPUT_GAMEPAD_LEFT_THUMB);
						bool rightThumb = (pad->wButtons & XINPUT_GAMEPAD_RIGHT_THUMB);
						bool leftShoulder = (pad->wButtons & XINPUT_GAMEPAD_LEFT_SHOULDER);
						bool rightShoulder = (pad->wButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER);
						bool aButton = (pad->wButtons & XINPUT_GAMEPAD_A);
						bool bButton = (pad->wButtons & XINPUT_GAMEPAD_B);
						bool xButton = (pad->wButtons & XINPUT_GAMEPAD_X);
						bool yButton = (pad->wButtons & XINPUT_GAMEPAD_Y);

						int16 stickX = pad->sThumbLX;
						int16 stickY = pad->sThumbLY;


						if ( up ) {
							yOffset += 4;
						}
						if ( down ) {
							yOffset -= 4;
						}
						if ( left ) {
							xOffset += 8;
						}
						if ( right ) {
							xOffset -= 8;
						}
					// controller not available
					} else {

					}

				}

				renderWeirdGradient(globalBackBuffer, xOffset, yOffset);

				HDC deviceContext = GetDC(windowHandle);
				win32_window_dimensions windowDimensions = getWindowDimensions(windowHandle);

				displayBufferInWindow(&globalBackBuffer, deviceContext, windowDimensions.width, windowDimensions.height);
				ReleaseDC(windowHandle, deviceContext);
				++xOffset;
			}
		}
	} else {
		// TODO: Logging
	}

	return 0;
}
