#if !defined(GAME_H)
#define GAME_H

#define global_variable static
#define internal static

inline void
Assert(bool stmt) 
{
  uint8 *NullPtr = 0;
  if (!stmt)
  {
    *NullPtr = 1;
  }
}

struct color
{
  uint8 R;
  uint8 G;
  uint8 B;
  uint8 A;
};

struct road_node 
{
  uint16 XCellIndex;
  uint16 YCellIndex;
  road_node *Next[8];
  uint8 NumNeighbours;
  bool Visited;
};

struct memory_arena 
{
  uint8 *Base;
  uint32 Size;
  uint32 Used;
};

struct target_buffer 
{
  pixel *Pixels;
  uint32 Width; // in pixels
  uint32 Height; // in pixels
};

struct game_state 
{
  uint8 HorizontalCellCount;
  uint8 VerticalCellCount;
  uint8 CellSideInPixels;
  color CellBorderColor;
  color BackgroundColor;
  uint8 CellBorderWidth;
  road_node **UnConnectedRoads;
  uint8 NumRoads;
  uint8 MaxNumRoads;
  // TODO(gaurav): Make this into a linked list of roadnode chunks
  road_node *RoadNodes;
  uint16 MaxNumRoadNodes;
  uint16 NumRoadNodes;
  target_buffer Buffer;
  bool IsInitialized;
  memory_arena Arena;
  bool MouseIsDown;
  uint8 MouseDownCellIndexX;
  uint8 MouseDownCellIndexY; 
};

#define PushStruct(Arena, struct_type) (struct_type *)PushSize_(Arena, sizeof(struct_type))
#define PushArray(Arena, count, struct_type) (struct_type *)PushSize_(Arena, (count)*sizeof(struct_type))
internal void *
PushSize_(memory_arena *Arena, uint32 Size)
{
  Assert(Size > 0);
  Assert(Arena->Used + Size < Arena->Size);
  void *Result = (void *)(Arena->Base + Arena->Used + Size);
  Arena->Used += Size;
  return Result;
}

inline color 
Color(uint8 R, uint8 G, uint8 B, uint8 A)
{
  color Result = { R, G, B, A };
  return Result;
}



#endif
