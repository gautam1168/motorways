#include "windows.h"

#define global_variable static
#define internal static
#define local_persist static

global_variable bool Running;
global_variable BITMAPINFO BitmapInfo;
global_variable void *BitmapMemory;
global_variable HBITMAP BitmapHandle;
global_variable HDC BitmapDeviceContext;

internal void
Win32ResizeDIBSection(int Width, int Height)
{
  if (BitmapHandle)
  {
    DeleteObject(BitmapHandle);
  }

  if (!BitmapDeviceContext)
  {
    BitmapDeviceContext = CreateCompatibleDC(0);
  }

  BitmapInfo.bmiHeader.biSize = sizeof(BitmapInfo.bmiHeader);
  BitmapInfo.bmiHeader.biWidth = Width;
  BitmapInfo.bmiHeader.biHeight = Height;
  BitmapInfo.bmiHeader.biPlanes = 1;
  BitmapInfo.bmiHeader.biBitCount = 32;
  BitmapInfo.bmiHeader.biCompression = BI_RGB;

  BitmapHandle = CreateDIBSection(
      BitmapDeviceContext,
      &BitmapInfo,
      DIB_RGB_COLORS,
      &BitmapMemory,
      0,
      0
    );
}

internal void
Win32UpdateWindow(
    HDC DeviceContext,
    int X,
    int Y,
    int Width,
    int Height)
{
  StretchDIBits(
      DeviceContext,
      X, Y, Width, Height,
      X, Y, Width, Height,
      BitmapMemory,
      &BitmapInfo,
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
      Win32ResizeDIBSection(Width, Height);
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
      int X = Paint.rcPaint.left;
      int Y = Paint.rcPaint.top;
      int Width = Paint.rcPaint.right - X;
      int Height = Paint.rcPaint.bottom - Y;

      local_persist WORD Operation = WHITENESS;
      PatBlt(DeviceContext, X, Y, Width, Height, Operation);

      /*
      if (Operation == WHITENESS)
      {
        Operation = BLACKNESS;
      }
      else
      {
        Operation = WHITENESS;
      }
      */
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
      MSG Message;
      Running = true;
      while (Running)
      {
        BOOL MessageResult = GetMessage(&Message, 0, 0, 0);
        if (MessageResult)
        {
          TranslateMessage(&Message);
          DispatchMessage(&Message);
        } 
        else
        {
          break;
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
