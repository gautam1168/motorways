#include "windows.h"
#include "stdint.h"

typedef uint8_t uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;

typedef int8_t int8;
typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;

#include "win32_game.h"
#include "game.h"
#include "game.cc"

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
global_variable int XOffset = 0, YOffset = 0;

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
InitializeGlobalBackBuffer(win32_offscreen_buffer *Buffer, game_memory *GameMemory, int Width, int Height)
{

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

  Buffer->Memory = GameMemory->BackbufferStorage;

  // RenderWeirdGradient(Buffer, 0, 0);
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
  win32_state Win32State = {};

  WNDCLASS WindowClass = {};
  WindowClass.style = CS_OWNDC|CS_HREDRAW|CS_VREDRAW;
  WindowClass.lpfnWndProc = MainWindowCallback;
  WindowClass.hInstance = Instance;
  WindowClass.lpszClassName = "Motorways Window";

  

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

    game_memory GameMemory = {};
    uint64 BufferWidth = 1280;
    uint64 BufferHeight = 720;
    uint64 BufferMemorySize = BufferWidth * BufferHeight * 4;
    GameMemory.PermanentStorageSize = Megabytes(4);
    GameMemory.TransientStorageSize = Megabytes(8);// Gigabytes((uint64)2);
    Win32State.TotalGameMemorySize = BufferMemorySize + GameMemory.PermanentStorageSize + GameMemory.TransientStorageSize;
    Win32State.GameMemoryBlock = VirtualAlloc(0, Win32State.TotalGameMemorySize, MEM_COMMIT|MEM_RESERVE, PAGE_READWRITE);

    GameMemory.BackbufferStorage = Win32State.GameMemoryBlock;
    GameMemory.PermanentStorage = ((uint8 *)Win32State.GameMemoryBlock + BufferMemorySize);
    GameMemory.TransientStorage = ((uint8 *)GameMemory.PermanentStorage + GameMemory.PermanentStorageSize);

    InitializeGlobalBackBuffer(&GlobalBackBuffer, &GameMemory, BufferWidth, BufferHeight);

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
        // RenderWeirdGradient(&GlobalBackBuffer, XOffset++, YOffset++);
        UpdateAndRender((uint8 *)Win32State.GameMemoryBlock, 
            128, 192, // Building
            64, 64, // House
            8, 14, // Car
            GlobalBackBuffer.Width, GlobalBackBuffer.Height,
            64, 64, false
            );
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
