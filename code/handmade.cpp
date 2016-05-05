// day 008 - direct sound write a square wave
#include <windows.h>
#include <stdint.h>
#include <math.h>
#include <xinput.h>
#include <dsound.h>

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
global_variable LPDIRECTSOUNDBUFFER globalSecondaryBuffer;

// XInputGetState wrapper to prevent null pointer
#define X_INPUT_GET_STATE(name) DWORD WINAPI name(DWORD dwUserIndex, XINPUT_STATE* pState)
typedef X_INPUT_GET_STATE(x_input_get_state);
X_INPUT_GET_STATE(XInputGetStateStub) {
	return ERROR_DEVICE_NOT_CONNECTED;
}
global_variable x_input_get_state *XInputGetState_ = XInputGetStateStub;

// XInputSetState wrapper to prevent null pointer
#define X_INPUT_SET_STATE(name) DWORD WINAPI name(DWORD dwUserIndex, XINPUT_VIBRATION* pVibration)
typedef X_INPUT_SET_STATE(x_input_set_state);
X_INPUT_SET_STATE(XInputSetStateStub) {
	return ERROR_DEVICE_NOT_CONNECTED;
}
global_variable x_input_set_state *XInputSetState_ = XInputSetStateStub;

typedef DWORD WINAPI x_input_get_state(DWORD dwUserIndex, XINPUT_STATE* pState);
typedef DWORD WINAPI x_input_set_state(DWORD dwUserIndex, XINPUT_VIBRATION* pVibration);

#define XInputGetState XInputGetState_
#define XInputSetState XInputSetState_


internal void loadXInput(void) {
	HMODULE XInputLibrary = LoadLibraryA("xinput1_3.dll");
	if ( !XInputLibrary ) {
		XInputLibrary = LoadLibraryA("xinput1_4.dll");
	}
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

#define DIRECT_SOUND_CREATE(name) HRESULT WINAPI name(LPCGUID pcGuidDevice, LPDIRECTSOUND *ppDS, LPUNKNOWN pUnkOuter )
typedef DIRECT_SOUND_CREATE(direct_sound_create);


internal void renderWeirdGradient(win32_offscreen_buffer *buffer, int xOffset, int  yOffset)  {

	uint8 *row = (uint8*)buffer->memory;
	for ( int y = 0; y < buffer->height; ++y ) {
		uint32 *pixel = (uint32*)row;
		for ( int x = 0; x < buffer->width; ++x ) {
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
		row += buffer->pitch;
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
	buffer->memory = VirtualAlloc(0, bitmapMemorySize, MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);

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

		case WM_SYSKEYDOWN:
		case WM_SYSKEYUP:
		case WM_KEYDOWN:
		case WM_KEYUP: {
			uint32 virtualKeyCode = wParam;
			bool wasDown = ( (lParam & (1<<30)) != 0 );
			bool isDown = ( (lParam & (1<<31)) == 0 );

			if ( wasDown != isDown  ) {

				if ( virtualKeyCode== 'W' ) {
				} else if ( virtualKeyCode== 'A' ) {
				} else if ( virtualKeyCode== 'S' ) {
				} else if ( virtualKeyCode== 'D' ) {
				} else if ( virtualKeyCode== 'Q' ) {
				} else if ( virtualKeyCode== 'E' ) {
				} else if ( virtualKeyCode== VK_UP ) {
				} else if ( virtualKeyCode== VK_LEFT ) {
				} else if ( virtualKeyCode== VK_DOWN ) {
				} else if ( virtualKeyCode== VK_RIGHT ) {
				} else if ( virtualKeyCode== VK_ESCAPE ) {
				} else if ( virtualKeyCode== VK_SPACE ) {
				}

				bool altKeyWasDown = (lParam & (1<<29)) != 0;
				if ( virtualKeyCode== VK_F4 && altKeyWasDown ) {
					running = false;
				}	
			}
			
			}break;
		case WM_PAINT:
		{
			PAINTSTRUCT paint;
			HDC deviceContext = BeginPaint(window,&paint);

			win32_window_dimensions windowDimensions = getWindowDimensions(window);

			displayBufferInWindow(&globalBackBuffer, deviceContext, windowDimensions.width,windowDimensions.height);
			EndPaint(window, &paint);
		}	break;

		default: {
			result = DefWindowProc(window, message, wParam, lParam);
			}break;
	}

	return result;
}

internal void initDirectSound(HWND windowHandle, int32 samplePerSecond, int32 bufferSize) {
	// Load the library
	HMODULE directSoundLibrary = LoadLibraryA("dsound.dll");
	if ( ! directSoundLibrary ) {
		OutputDebugStringA("Direct sound dll not found ");
		return;
	}

	// Get a direct sound object
	direct_sound_create *directSoundCreate = (direct_sound_create*)GetProcAddress(directSoundLibrary, "DirectSoundCreate");

	HRESULT error;

	LPDIRECTSOUND directSound;
	error = directSoundCreate(0, &directSound, 0);
	if ( directSoundCreate && SUCCEEDED(error) ) {

		DSBUFFERDESC bufferDescription = {};
		bufferDescription.dwSize = sizeof(bufferDescription);
		bufferDescription.dwFlags = DSBCAPS_PRIMARYBUFFER;

		WAVEFORMATEX waveFormat = {};
		waveFormat.wFormatTag = WAVE_FORMAT_PCM;
		waveFormat.nChannels = 2;
		waveFormat.nSamplesPerSec = samplePerSecond;
		waveFormat.wBitsPerSample = 16;
		waveFormat.nBlockAlign = (waveFormat.nChannels*waveFormat.wBitsPerSample) / 8;
		waveFormat.nAvgBytesPerSec = waveFormat.nSamplesPerSec * waveFormat.nBlockAlign; 
		waveFormat.cbSize = 0;

		

		LPDIRECTSOUNDBUFFER primaryBuffer;
		// create a primary buffer
		
		if ( SUCCEEDED(directSound->SetCooperativeLevel(windowHandle, DSSCL_PRIORITY)) ) {

			if ( SUCCEEDED(error = directSound->CreateSoundBuffer(&bufferDescription, &primaryBuffer, 0)) ) {
				
				if ( !(SUCCEEDED(error = primaryBuffer->SetFormat(&waveFormat))) ) {
					OutputDebugStringA("Primary buffer failed " + error );
				}

			} else {
				OutputDebugStringA("Primary buffer failed " + error);
			}

			DSBUFFERDESC bufferDescription = {};
			bufferDescription.dwSize = sizeof(bufferDescription);
			bufferDescription.dwFlags = 0;
			bufferDescription.dwBufferBytes = bufferSize;
			bufferDescription.lpwfxFormat = &waveFormat;

			
			// create a secondary buffer
			if ( SUCCEEDED(directSound->CreateSoundBuffer(&bufferDescription, &globalSecondaryBuffer, 0)) ) {

			}
		}

		

		// start playing it
	}
	else {
		OutputDebugStringA("Create DSound error " + error);
	}

	

}

int CALLBACK WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, 
	LPSTR lpCmdLine, int nCmdShow){

	loadXInput();

	WNDCLASSA windowClass = {};

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

			// graphics test
			int xOffset = 0;
			int yOffset = 0;


			// sound test
			int samplesPerSecond = 48000;
			uint32 runningSampleIndex = 0;
			int toneHz = 256;
			int bytesPerSample = sizeof(int16)*2;
			int squareWavePeriod = samplesPerSecond/toneHz;
			int halfSquareWavePeriod = squareWavePeriod / 2;
			int secondaryBufferSize = samplesPerSecond*bytesPerSample;

			initDirectSound(windowHandle, samplesPerSecond, secondaryBufferSize);
			globalSecondaryBuffer->Play(0, 0, DSBPLAY_LOOPING);

			while ( running ) {

				MSG message;
				while ( PeekMessage(&message, 0, 0, 0, PM_REMOVE) ) {

					if ( message.message == WM_QUIT ) {
						running = false;
					}

					TranslateMessage(&message);
					DispatchMessage(&message);
				}

				// TODO: should we poll this more frequently?
				for ( DWORD controllerIdx = 0; controllerIdx < XUSER_MAX_COUNT; ++controllerIdx ) {
					XINPUT_STATE controllerState;
					
					// this controller is plugged in
					if ( XInputGetState(controllerIdx, &controllerState) == ERROR_SUCCESS ) {

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


						
						xOffset -= stickX>>12;
						yOffset += stickY>>12;
						
						if ( xButton ) {
							XINPUT_VIBRATION vibration;
							vibration.wLeftMotorSpeed = 65534;
							vibration.wRightMotorSpeed = 65534;
							XInputSetState(0, &vibration);
						}

					// controller not available
					} else {

					}

				}
				

				renderWeirdGradient(&globalBackBuffer, xOffset, yOffset);

				

				DWORD playCursor;
				DWORD writeCursor;

				if ( SUCCEEDED(globalSecondaryBuffer->GetCurrentPosition(&playCursor, &writeCursor)) ) {



					// Samples:
					//  16bit 16bit, etc..
					// [LEFT  RIGHT] LEFT RIGHT LEFT RIGHT .... 
					// direct sound output test:
					VOID *region1;
					DWORD region1Size;
					VOID *region2;
					DWORD region2Size;

					DWORD byteToLock = (runningSampleIndex*bytesPerSample) % secondaryBufferSize;
					DWORD bytesToWrite;
					if ( byteToLock > playCursor ) {
						bytesToWrite = (secondaryBufferSize - byteToLock);
						bytesToWrite += playCursor;
					} else {
						bytesToWrite = playCursor - byteToLock;
					}
					if ( SUCCEEDED(globalSecondaryBuffer->Lock(byteToLock,bytesToWrite,
																&region1, &region1Size,
																&region2, &region2Size, 0)) ) {
						int16 *sampleOut = (int16*)region1;
						DWORD region1SampleCount = region1Size/bytesPerSample;
						for ( DWORD sampleIndex = 0; sampleIndex < region1SampleCount; ++sampleIndex ) {
							int16 sampleValue = ((runningSampleIndex++ / halfSquareWavePeriod) % 2) ? 16000 : -16000;
							*sampleOut++ = sampleValue;
							*sampleOut++ = sampleValue;
						}

						DWORD region2SampleCount = region2Size/bytesPerSample;
						sampleOut = (int16*)region2;
						for ( DWORD sampleIndex = 0; sampleIndex < region2SampleCount; ++sampleIndex ) {
							int16 sampleValue = ((runningSampleIndex++ / halfSquareWavePeriod) % 2) ? 16000 : -16000;
							*sampleOut++ = sampleValue;
							*sampleOut++ = sampleValue;
						}
					}
				}


				HDC deviceContext = GetDC(windowHandle);
				win32_window_dimensions windowDimensions = getWindowDimensions(windowHandle);

				displayBufferInWindow(&globalBackBuffer, deviceContext, windowDimensions.width, windowDimensions.height);
				ReleaseDC(windowHandle, deviceContext);
			}
		}
	} else {
		// TODO: Logging
	}

	return 0;
}
