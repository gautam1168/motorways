#include "windows.h"
#include "stdint.h"

#define global_variable static
#define internal static
#define local_persist static

typedef uint8_t uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;

typedef int8_t int8;
typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;

struct win32_offscreen_buffer
{
  BITMAPINFO BitmapInfo;
  void *Memory;
  int Width;
  int Height;
  int BytesPerPixel; 
  int Pitch;
};

struct win32_window_dimensions 
{
  int Width;
  int Height;
};

global_variable bool Running;
global_variable win32_offscreen_buffer GlobalBackBuffer;

internal win32_window_dimensions 
Win32GetWindowDimensions(HWND Window)
{
  win32_window_dimensions Result;
  RECT ClientRect;
  GetClientRect(Window, &ClientRect);
  Result.Width = ClientRect.right - ClientRect.left;
  Result.Height = ClientRect.bottom - ClientRect.top;

  return Result;
}

internal void
RenderWeirdGradient(win32_offscreen_buffer *Buffer, int XOffset, int YOffset)
{
  uint8 *Row = (uint8 *)Buffer->Memory;
  for (int Y = 0; Y < Buffer->Height; ++Y)
  {
    uint32 *Pixel = (uint32 *)Row;
    for (int X = 0; X < Buffer->Width; ++X)
    {
      uint8 Blue = (X + XOffset);
      uint8 Green = (Y + YOffset);
      *Pixel++ = (Green << 8) | Blue;
    }
    Row += Buffer->Pitch;
  }
}

internal void
Win32ResizeDIBSection(win32_offscreen_buffer *Buffer, int Width, int Height)
{
  if (Buffer->Memory)
  {
    VirtualFree(Buffer->Memory, 0, MEM_RELEASE);
  }

  Buffer->Width = Width;
  Buffer->Height = Height;
  Buffer->BytesPerPixel = 4;
  Buffer->Pitch = Width * Buffer->BytesPerPixel;

  Buffer->BitmapInfo.bmiHeader.biSize = sizeof(Buffer->BitmapInfo.bmiHeader);
  Buffer->BitmapInfo.bmiHeader.biWidth = Buffer->Width;
  Buffer->BitmapInfo.bmiHeader.biHeight = -Buffer->Height;
  Buffer->BitmapInfo.bmiHeader.biPlanes = 1;
  Buffer->BitmapInfo.bmiHeader.biBitCount = 32;
  Buffer->BitmapInfo.bmiHeader.biCompression = BI_RGB;

  int BitmapMemorySize = Buffer->BytesPerPixel * Buffer->Width * Buffer->Height;
  Buffer->Memory = VirtualAlloc(0, BitmapMemorySize, MEM_COMMIT, PAGE_READWRITE);

  RenderWeirdGradient(Buffer, 0, 0);
}

internal void
Win32DisplayBufferInWindow(
    HDC DeviceContext,
    win32_window_dimensions WinDims,
    win32_offscreen_buffer *Buffer
    )
{
  StretchDIBits(
      DeviceContext,
      /*
      X, Y, Width, Height,
      X, Y, Width, Height,
      */
      0, 0, Buffer->Width, Buffer->Height,
      0, 0, WinDims.Width, WinDims.Height,
      Buffer->Memory,
      &Buffer->BitmapInfo,
      DIB_RGB_COLORS,
      SRCCOPY
      );
}

LRESULT CALLBACK MainWindowCallback(
    HWND Window, 
    UINT Message, 
    WPARAM wParam, 
    LPARAM lParam)
{
  LRESULT Result = 0;

  switch (Message)
  {
    case WM_SIZE:
    {
      RECT ClientRect;
      GetClientRect(Window, &ClientRect);
      int Width = ClientRect.right - ClientRect.left;
      int Height = ClientRect.bottom - ClientRect.top;
      Win32ResizeDIBSection(&GlobalBackBuffer, Width, Height);
    } break;
    case WM_DESTROY:
    {
      Running = false;
    } break;
    case WM_CLOSE:
    {
      Running = false;
    } break;
    case WM_PAINT:
    {
      PAINTSTRUCT Paint;
      HDC DeviceContext = BeginPaint(Window, &Paint);
      win32_window_dimensions WinDims = Win32GetWindowDimensions(Window);
      Win32DisplayBufferInWindow(DeviceContext, WinDims, &GlobalBackBuffer);

      EndPaint(Window, &Paint);
    } break;

    default:
    {
      OutputDebugString("default\n");
      Result = DefWindowProc(Window, Message, wParam, lParam);
    }
  }

  return Result;
}

int WINAPI WinMain(
    HINSTANCE Instance, 
    HINSTANCE PrevInstance,
    PSTR CommandLine, 
    INT ShowCode)
{
  WNDCLASS WindowClass = {};
  WindowClass.style = CS_OWNDC|CS_HREDRAW|CS_VREDRAW;
  WindowClass.lpfnWndProc = MainWindowCallback;
  WindowClass.hInstance = Instance;
  WindowClass.lpszClassName = "Motorways Window";

  Win32ResizeDIBSection(&GlobalBackBuffer, 1280, 720);

  if (RegisterClassA(&WindowClass))
  {
    HWND WindowHandle = CreateWindowExA(
        0,
        WindowClass.lpszClassName,
        "Motorways",
        WS_OVERLAPPEDWINDOW|WS_VISIBLE,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        0,
        0,
        Instance,
        0
      );

    if (WindowHandle)
    {
      int XOffset = 0, YOffset = 0;
      MSG Message;
      Running = true;
      while (Running)
      {
        BOOL MessageResult = GetMessage(&Message, 0, 0, 0);
        if (MessageResult)
        {
          if (Message.message == WM_QUIT)
          {
            Running = false;
          }
          TranslateMessage(&Message);
          DispatchMessage(&Message);
        } 
        RenderWeirdGradient(&GlobalBackBuffer, XOffset++, YOffset++);
        {
          HDC DeviceContext = GetDC(WindowHandle);
          win32_window_dimensions WinDims = Win32GetWindowDimensions(WindowHandle);
          Win32DisplayBufferInWindow(DeviceContext, WinDims, &GlobalBackBuffer);
          ReleaseDC(WindowHandle, DeviceContext);
        }
      }
    }
    else 
    {
      OutputDebugString("No window handle was obtained from createWindow");
    }
  }
  else
  {
    OutputDebugString("Registration of window class failed");
  }

  return 0;
}
