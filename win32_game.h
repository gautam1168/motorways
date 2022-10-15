#if !defined(WIN32_GAME_H)
#define WIN32_GAME_H

union pixel 
{
  struct 
  {

    uint8 B;
    uint8 G;
    uint8 R;
    uint8 A;
  };

  uint32 Color;
};

struct win32_state 
{
  uint64 TotalGameMemorySize;
  void *GameMemoryBlock;
};

#endif
