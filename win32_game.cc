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

typedef float real32;
typedef double real64;

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

struct debug_read_file_result
{
  uint32 ContentSize;
  void *Contents;
};

#pragma pack(push, 1)
struct bitmap_header
{
  uint16 FileType;
  uint32 FileSize;
  uint16 Reserved1;
  uint16 Reserved2;
  uint32 BitmapOffset;
  uint32 Size;
  uint32 Width;
  uint32 Height;
  uint16 Planes;
  uint16 BitsPerPixel;
  uint32 Compression;
  uint32 ImageSize;
  uint32 XPixelsPerM;
  uint32 YPixelsPerM;
  uint32 ColorsUsed;
  uint32 ImportantColors;
  uint32 RedMask;
  uint32 GreenMask;
  uint32 BlueMask;
  uint32 AlphaMask;
};
#pragma pack(pop)

struct loaded_bitmap
{
  uint32 *Pixels;
  uint32 Width;
  uint32 Height;
};

global_variable bool Running, MouseIsDown = false;
global_variable win32_offscreen_buffer GlobalBackBuffer;
global_variable uint16 MouseX = 0, MouseY = 0;

internal void
LoadBitmapIntoGameMemory(game_memory *GameMemory, uint64 Offset, uint64 Width, uint64 Height, char *FileName)
{
  debug_read_file_result Result = {};
  HANDLE FileHandle = CreateFileA(FileName, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);
  if (FileHandle != INVALID_HANDLE_VALUE)
  {
    LARGE_INTEGER FileSize;
    if (GetFileSizeEx(FileHandle, &FileSize))
    {
      uint32 FileSize32 = SafeTruncateUint64(FileSize.QuadPart);    
      Result.Contents = VirtualAlloc(0, FileSize32, MEM_COMMIT|MEM_RESERVE, PAGE_READWRITE);
      if (Result.Contents)
      {
        DWORD BytesRead;
        if (ReadFile(FileHandle, Result.Contents, FileSize32, &BytesRead, 0) && BytesRead == FileSize32) 
        {
          Result.ContentSize = FileSize32;

          bitmap_header *Header = (bitmap_header *)Result.Contents;
          loaded_bitmap LoadedBMP = {};
          LoadedBMP.Pixels = (uint32 *)GameMemory->PermanentStorage + Offset;
          LoadedBMP.Width = Header->Width;
          LoadedBMP.Height = Header->Height;
          Assert(Header->Width == Width);
          Assert(Header->Height == Height);

          Assert((Header->AlphaMask | Header->GreenMask | Header->RedMask | Header->BlueMask) == 0xffffffff);
          uint32 RedShift = 0;
          uint32 BlueShift = 0;
          uint32 GreenShift = 0;
          uint32 AlphaShift = 0;

          Assert(FindLowestSetBit(&RedShift, Header->RedMask));
          Assert(FindLowestSetBit(&BlueShift, Header->BlueMask));
          Assert(FindLowestSetBit(&GreenShift, Header->GreenMask));
          Assert(FindLowestSetBit(&AlphaShift, Header->AlphaMask));

          uint32 *Source = (uint32 *)((uint8 *)Result.Contents + Header->BitmapOffset);
          pixel *Dest = (pixel *)LoadedBMP.Pixels;
          Assert(Header->Height == Height);
          Assert(Header->Width == Width);
          for (int32 Y = Height - 1;
              Y >= 0;
              --Y)
          {
            for (uint32 X = 0;
                X < Width;
                ++X)
            {
              uint32 C = *(Source + (Width * Y) + X);
              uint8 R = ((C >> RedShift) & 0xff);
              uint8 G = ((C >> GreenShift) & 0xff);
              uint8 B = ((C >> BlueShift) & 0xff);
              uint8 A = ((C >> AlphaShift) & 0xff);

              Dest->R = R;
              Dest->G = G;
              Dest->B = B;
              Dest->A = A;
              ++Dest;
            }
          }

          VirtualFree(Result.Contents, 0, MEM_RELEASE);
        }
        else
        {
          if (Result.Contents)
          {
            VirtualFree(Result.Contents, 0, MEM_RELEASE);
          }
          Result.Contents = 0;
          Result.ContentSize = 0;
        }
      }
    }
    CloseHandle(FileHandle);
  }
  else
  {
    OutputDebugString("Error when reading file!\n");
  }
}

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

    uint64 CarWidth = 8, CarHeight = 14, BuildingWidth = 128, BuildingHeight = 192, HouseWidth = 64, HouseHeight = 64;
    InitializeGlobalBackBuffer(&GlobalBackBuffer, &GameMemory, BufferWidth, BufferHeight);
    uint64 Offset = 0;
    LoadBitmapIntoGameMemory(&GameMemory, Offset, BuildingWidth, BuildingHeight, ".\\buildingpink.bmp");
    // LoadBitmapIntoGameMemory(&GameMemory, Offset, 60, 40, ".\\structured_art.bmp");
    Offset += BuildingWidth * BuildingHeight;
    LoadBitmapIntoGameMemory(&GameMemory, Offset, HouseWidth, HouseHeight, ".\\housepink.bmp");
    Offset += HouseWidth * HouseHeight;
    LoadBitmapIntoGameMemory(&GameMemory, Offset, CarWidth, CarHeight, ".\\carpink.bmp");

    if (WindowHandle)
    {
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

        POINT MouseP;
        GetCursorPos(&MouseP);
        ScreenToClient(WindowHandle, &MouseP);
        MouseX = MouseP.x;
        MouseY = MouseP.y;
        if (GetKeyState(VK_LBUTTON) & (1 << 15))
        {
          MouseIsDown = true;
        }
        else
        {
          MouseIsDown = false;
        }

        UpdateAndRender((uint8 *)Win32State.GameMemoryBlock, 
            128, 192, // Building
            64, 64, // House
            8, 14, // Car
            GlobalBackBuffer.Width, GlobalBackBuffer.Height,
            MouseX, MouseY, MouseIsDown
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
