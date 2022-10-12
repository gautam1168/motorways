#include "game.h"

global_variable game_state GameState;

internal void
FillColor(game_state *GameState, uint16 Top, uint16 Left, 
    uint16 Width, uint16 Height, color Color)
{
  target_buffer *Buffer = &GameState->Buffer;
  pixel *Pixels = Buffer->Pixels;
  for (uint16 XIndex = Left;
      XIndex < Left + Width;
      ++XIndex)
  {
    for (uint16 YIndex = Top;
        YIndex < Top + Height;
        ++YIndex)
    {
      uint32 PixelIndex = XIndex + (Buffer->Width * YIndex);
      if (PixelIndex < Buffer->Width * Buffer->Height) 
      {
        pixel *Pixel = Pixels + PixelIndex;
        Pixel->R = Color.R;
        Pixel->G = Color.G;
        Pixel->B = Color.B;
        Pixel->A = Color.A;
      }
    }
  }
}

internal void
DrawCircle(game_state *GameState, 
    uint16 CenterX, uint16 CenterY, uint16 Radius, color Color, 
    uint16 LineWidth)
{
  target_buffer *Buffer = &GameState->Buffer;
  pixel *Pixels = Buffer->Pixels;
  uint32 R2External = Radius * Radius;
  uint32 R2Internal = (Radius - LineWidth) * (Radius - LineWidth);

  for (uint16 XIndex = CenterX - Radius; 
      XIndex < CenterX + Radius;
      ++XIndex)
  {
    for (uint16 YIndex = CenterY - Radius;
        YIndex < CenterY + Radius;
        ++YIndex)
    {
      uint32 Dx2 = (CenterX - XIndex) * (CenterX - XIndex);
      uint32 Dy2 = (CenterY - YIndex) * (CenterY - YIndex);
      
      if (Dx2 + Dy2 < R2External && Dx2 + Dy2 > R2Internal) {
        uint32 PixelIndex = XIndex + (Buffer->Width * YIndex);
        if (PixelIndex < Buffer->Width * Buffer->Height) 
        {
          pixel *Pixel = Pixels + PixelIndex;
          Pixel->R = Color.R;
          Pixel->G = Color.G;
          Pixel->B = Color.B;
          Pixel->A = Color.A;
        }
      }
      else if (Dx2 + Dy2 - R2External <= 1) 
      {
        uint32 PixelIndex = XIndex + (Buffer->Width * YIndex);
        if (PixelIndex < Buffer->Width * Buffer->Height) 
        {
          pixel *Pixel = Pixels + PixelIndex;
          Pixel->R = 0x77;//(uint8)((Pixel->R * 0.5f) + (Color.R * 0.5f));
          Pixel->G = 0x78;//(uint8)((Pixel->G * 0.5f) + (Color.G * 0.5f));
          Pixel->B = 0x85;//(uint8)((Pixel->B * 0.5f) + (Color.B * 0.5f));
          Pixel->A = 0xff; //0x88;
        }
      }
      else if (Dx2 + Dy2 - R2Internal <= 1) 
      {
        uint32 PixelIndex = XIndex + (Buffer->Width * YIndex);
        if (PixelIndex < Buffer->Width * Buffer->Height) 
        {
          pixel *Pixel = Pixels + PixelIndex;
          Pixel->R = 0x77;//(uint8)((Pixel->R * 0.5f) + (Color.R * 0.5f));
          Pixel->G = 0x78;//(uint8)((Pixel->G * 0.5f) + (Color.G * 0.5f));
          Pixel->B = 0x85;//(uint8)((Pixel->B * 0.5f) + (Color.B * 0.5f));
          Pixel->A = 0xff; //0x88;
        }
      }
    }
  }

}

internal void
DrawFilledCircle(game_state *GameState, uint16 CenterX, uint16 CenterY, 
    uint16 Radius, color Color) 
{
  target_buffer *Buffer = &GameState->Buffer;
  pixel *Pixels = Buffer->Pixels;
  uint32 R2 = Radius * Radius;

  for (uint16 XIndex = CenterX - Radius; 
      XIndex < CenterX + Radius;
      ++XIndex)
  {
    for (uint16 YIndex = CenterY - Radius;
        YIndex < CenterY + Radius;
        ++YIndex)
    {
      uint32 Dx2 = (CenterX - XIndex) * (CenterX - XIndex);
      uint32 Dy2 = (CenterY - YIndex) * (CenterY - YIndex);
      
      if (Dx2 + Dy2 < R2) {
        uint32 PixelIndex = XIndex + (Buffer->Width * YIndex);
        if (PixelIndex < Buffer->Width * Buffer->Height) 
        {
          pixel *Pixel = Pixels + PixelIndex;
          Pixel->R = Color.R;
          Pixel->G = Color.G;
          Pixel->B = Color.B;
          Pixel->A = Color.A;
        }
      } 
      else if (Dx2 + Dy2 - R2 <= 1) 
      {
        uint32 PixelIndex = XIndex + (Buffer->Width * YIndex);
        if (PixelIndex < Buffer->Width * Buffer->Height) 
        {
          pixel *Pixel = Pixels + PixelIndex;
          Pixel->R = 0x77;//(uint8)((Pixel->R * 0.5f) + (Color.R * 0.5f));
          Pixel->G = 0x78;//(uint8)((Pixel->G * 0.5f) + (Color.G * 0.5f));
          Pixel->B = 0x85;//(uint8)((Pixel->B * 0.5f) + (Color.B * 0.5f));
          Pixel->A = 0xff; //0x88;
        }
      }
    }
  }
}

internal void
DrawBox(game_state *GameState, 
    uint16 Top, uint16 Left, uint16 Width, uint16 Height, color Color, 
    uint16 LineWidth)
{
  // Draw top
  uint16 BoxLeft = Left;
  uint16 BoxTop = Top;
  uint16 BoxWidth = Width;
  uint16 BoxHeight = LineWidth;
  FillColor(GameState, BoxTop, BoxLeft, BoxWidth, BoxHeight, Color);

  // Draw left
  BoxLeft = Left;
  BoxTop = Top;
  BoxWidth = LineWidth;
  BoxHeight = Height;
  FillColor(GameState, BoxTop, BoxLeft, BoxWidth, BoxHeight, Color);

  // Draw bottom
  BoxLeft = Left;
  BoxTop = Top + Height - LineWidth;
  BoxWidth = Width;
  BoxHeight = LineWidth;
  FillColor(GameState, BoxTop, BoxLeft, BoxWidth, BoxHeight, Color);


  // Draw right
  BoxLeft = Left + Width - LineWidth;
  BoxTop = Top;
  BoxWidth = LineWidth;
  BoxHeight = Height;
  FillColor(GameState, BoxTop, BoxLeft, BoxWidth, BoxHeight, Color);
}

internal void
DrawGrid(game_state *GameState)
{
  for (uint8 VerticalCellIndex = 0;
      VerticalCellIndex < GameState->VerticalCellCount;
      ++VerticalCellIndex)
  {
    for (uint8 HorizontalCellIndex = 0;
        HorizontalCellIndex < GameState->HorizontalCellCount;
        ++HorizontalCellIndex)
    {
      uint16 Left = HorizontalCellIndex * GameState->CellSideInPixels;
      uint16 Top = VerticalCellIndex * GameState->CellSideInPixels;
      uint16 Width = GameState->CellSideInPixels;
      uint16 Height = GameState->CellSideInPixels;
      DrawBox(GameState, Top, Left, Width, 
          Height, GameState->CellBorderColor, 
          GameState->CellBorderWidth);
    }
  }
}

internal void
DrawImage(game_state *GameState, pixel *ImageBuffer, uint16 ImageWidth, 
    uint16 ImageHeight, uint16 BufferWidth, uint16 CellIndexX, 
    uint16 CellIndexY, uint16 OffsetX = 0, uint16 OffsetY = 0)
{
  pixel *SourcePixel = ImageBuffer;
  uint16 CellSideInPixels = GameState->CellSideInPixels;

  for (uint16 YIndex = CellIndexY * CellSideInPixels;
        YIndex < ImageHeight + (CellIndexY * CellSideInPixels);
        ++YIndex)
   {
    for (uint16 XIndex = CellIndexX * CellSideInPixels; 
      XIndex < ImageWidth + CellIndexX * CellSideInPixels;
      ++XIndex)
    {  

      pixel *Pixel = GameState->Buffer.Pixels + BufferWidth*(YIndex + OffsetY) + (XIndex + OffsetX);
      if (SourcePixel->A == 0xff) {
        Pixel->R = SourcePixel->R;
        Pixel->G = SourcePixel->G;
        Pixel->B = SourcePixel->B;
      } 
      SourcePixel++;
    }
  }
}

internal void
DrawRoads(game_state *GameState) 
{
  for (uint8 RoadIndex = 0;
      RoadIndex < GameState->NumRoads;
      ++RoadIndex)
  {
    target_buffer *Buffer = &GameState->Buffer;
    road_node *RoadStart = GameState->UnConnectedRoads[RoadIndex];
    // Do a dfs here
    
    road_node *Stack[50];
    uint8 StackEmptySlotIndex = 0;
    uint8 CellSideInPixels = GameState->CellSideInPixels;
    color CellBorderColor = GameState->CellBorderColor;

    Stack[StackEmptySlotIndex++] = RoadStart;
    while (StackEmptySlotIndex > 0)
    {
      road_node *Start = Stack[--StackEmptySlotIndex];
      road_node *End;
      Start->Visited = true;

      DrawFilledCircle(GameState, 
          (uint16)(RoadStart->XCellIndex * CellSideInPixels + 0.5 * CellSideInPixels), 
          (uint16)(RoadStart->YCellIndex * CellSideInPixels + 0.5 * CellSideInPixels),
          (uint16)(0.25f*CellSideInPixels), CellBorderColor);

      for (uint8 NeighbourIndex = 0;
          NeighbourIndex < Start->NumNeighbours;
          ++NeighbourIndex)
      {
        road_node *Neighbour = Start->Next[NeighbourIndex];

        if (Neighbour->XCellIndex == Start->XCellIndex + 1 && 
            Neighbour->YCellIndex == Start->YCellIndex) 
        {
          // Right neighbour
          if (!Neighbour->Visited) 
          {
            Stack[StackEmptySlotIndex++] = Neighbour;
          }

          FillColor(GameState, 
              Start->YCellIndex*CellSideInPixels + (uint16)(0.25f * CellSideInPixels), 
              Start->XCellIndex*CellSideInPixels + (uint16)(0.5f * CellSideInPixels),
              CellSideInPixels, (uint16)(0.5f * CellSideInPixels), CellBorderColor);
        }
        else if ( Neighbour->XCellIndex == Start->XCellIndex &&
            Neighbour->YCellIndex == Start->YCellIndex + 1) 
        {
          // Bottom neighbour
          if (!Neighbour->Visited) 
          {
            Stack[StackEmptySlotIndex++] = Neighbour;
          }

          FillColor(GameState, 
              Start->YCellIndex*CellSideInPixels + (uint16)(0.5f * CellSideInPixels), 
              Start->XCellIndex*CellSideInPixels + (uint16)(0.25f * CellSideInPixels),
              (uint16)(0.5f * CellSideInPixels), CellSideInPixels, CellBorderColor);
        } else if (Neighbour->XCellIndex + 1 == Start->XCellIndex &&
            Neighbour->YCellIndex == Start->YCellIndex)
        {
          // Left neighbour
          if (!Neighbour->Visited) 
          {
            Stack[StackEmptySlotIndex++] = Neighbour;
          }

          FillColor(GameState, 
              Neighbour->YCellIndex*CellSideInPixels + (uint16)(0.25f * CellSideInPixels), 
              Neighbour->XCellIndex*CellSideInPixels + (uint16)(0.5f * CellSideInPixels),
              CellSideInPixels, (uint16)(0.5f * CellSideInPixels), CellBorderColor);
        } else if (Neighbour->XCellIndex == Start->XCellIndex &&
            Neighbour->YCellIndex + 1 == Start->YCellIndex)
        {
          // Bottom neighbour
          if (!Neighbour->Visited) 
          {
            Stack[StackEmptySlotIndex++] = Neighbour;
          }

          FillColor(GameState, 
              Neighbour->YCellIndex*CellSideInPixels + (uint16)(0.5f * CellSideInPixels), 
              Neighbour->XCellIndex*CellSideInPixels + (uint16)(0.25f * CellSideInPixels),
              (uint16)(0.5f * CellSideInPixels), CellSideInPixels, CellBorderColor);
        }


        DrawFilledCircle(GameState, 
          (uint16)(Neighbour->XCellIndex * CellSideInPixels + 0.5 * CellSideInPixels), 
          (uint16)(Neighbour->YCellIndex * CellSideInPixels + 0.5 * CellSideInPixels),
          (uint16)(0.25f*CellSideInPixels), CellBorderColor);
      }
    }

    for (uint32 RoadNodeIndex = 0;
          RoadNodeIndex < GameState->NumRoadNodes;
          ++RoadNodeIndex)
    {
      road_node *RoadNode = GameState->RoadNodes + RoadNodeIndex;
      RoadNode->Visited = false;
    }
  }
}

inline uint16
Min(uint16 A, uint16 B)
{
  uint16 Result = A < B ? A : B;
  return Result;
}

inline uint16
Max(uint16 A, uint16 B)
{
  uint16 Result = A > B ? A : B;
  return Result;
}

inline uint16
Square(uint16 Val)
{
  uint16 Result = Val * Val;
  return Result;
}

internal void
DrawLine(game_state *GameState, uint16 StartX, uint16 StartY, 
    uint16 EndX, uint16 EndY, color Color)
{
  uint16 BoundingBoxX1 = Min(StartX, EndX) - 10;
  uint16 BoundingBoxX2 = Max(StartX, EndX) + 10;
  uint16 BoundingBoxY1 = Min(StartY, EndY) - 10;
  uint16 BoundingBoxY2 = Max(StartY, EndY) + 10;

  float Slope = (float)(EndY - StartY)/
    (float)(EndX - StartX);
  
  for (uint16 YIndex = BoundingBoxY1; 
      YIndex < BoundingBoxY2;
      ++YIndex)
  {
    for (uint16 XIndex = BoundingBoxX1;
        XIndex < BoundingBoxX2;
        ++XIndex)
    {
      uint16 LineY = StartY + Slope * (XIndex - StartX);
      if (YIndex - LineY > -20 && 
          YIndex - LineY < 20)
      {
        pixel *Pixel = GameState->Buffer.Pixels + 
          YIndex * GameState->Buffer.Width + XIndex;
        Pixel->R = Color.R;
        Pixel->G = Color.G;
        Pixel->B = Color.B;
        Pixel->A = Color.A;
      }
    }
  }
}

internal road_node *
FindRoadNode(game_state *GameState, uint16 XIndex, uint16 YIndex)
{
  road_node *Result = 0;
  for (uint16 RoadNodeIndex = 0;
      RoadNodeIndex < GameState->NumRoadNodes;
      ++RoadNodeIndex)
  {
    road_node *RoadNode = GameState->RoadNodes + RoadNodeIndex;
    if ((RoadNode->XCellIndex == XIndex) && 
        (RoadNode->YCellIndex == YIndex))
    {
      Result = RoadNode;
      break;
    }
  }
  return Result;
}

internal void
CreateAndInsertNodes(game_state *GameState, 
    uint16 StartXIndex, uint16 StartYIndex,
    uint16 EndXIndex, uint16 EndYIndex)
{
  road_node *StartNode = FindRoadNode(GameState, StartXIndex, StartYIndex);
  road_node *EndNode = FindRoadNode(GameState, EndXIndex, EndYIndex);

  bool ShouldAddUnconnectedRoad = !StartNode && !EndNode;
  
  if (!StartNode)
  {
    StartNode = GameState->RoadNodes + GameState->NumRoadNodes++;
    StartNode->XCellIndex = StartXIndex;
    StartNode->YCellIndex = StartYIndex;
    StartNode->Visited = false;
    StartNode->NumNeighbours = 0;
  }

  if (!EndNode)
  {
    EndNode = GameState->RoadNodes + GameState->NumRoadNodes++;
    EndNode->XCellIndex = EndXIndex;
    EndNode->YCellIndex = EndYIndex;
    EndNode->Visited = false;
    EndNode->NumNeighbours = 0;
  }

  bool NeighbourAlreadyPresent = false;
  for (uint8 NeighbourIndex = 0;
      NeighbourIndex < StartNode->NumNeighbours;
      ++NeighbourIndex)
  {
    road_node *Neighbour = StartNode->Next[NeighbourIndex];
    if (Neighbour)
    {
      NeighbourAlreadyPresent = (Neighbour->XCellIndex == EndNode->XCellIndex) &&
        (Neighbour->YCellIndex == EndNode->YCellIndex);
    }
  }

  if (!NeighbourAlreadyPresent)
  {
    StartNode->Next[StartNode->NumNeighbours++] = EndNode;
  }

  if (ShouldAddUnconnectedRoad) 
  {
    GameState->UnConnectedRoads[GameState->NumRoads++] = StartNode;
  }
}

internal void
DrawHouses(game_state *GameState) 
{
  for (uint32 HouseIndex = 0;
      HouseIndex < GameState->MaxNumHouses;
      ++HouseIndex)
  {
    house *House = GameState->Houses + HouseIndex;
    DrawImage(GameState, House->Pixels, House->Width, House->Height, GameState->Buffer.Width, 
        House->CellIndexX, House->CellIndexY);
  }
}

internal void
DrawBuildings(game_state *GameState)
{
  for (uint32 BuildingIndex = 0;
      BuildingIndex < GameState->MaxNumBuildings;
      ++BuildingIndex)
  {
    building *Building = GameState->Buildings + BuildingIndex;
    DrawImage(GameState, Building->Pixels, Building->Width, Building->Height, GameState->Buffer.Width, 
        Building->CellIndexX, Building->CellIndexY);
  }

}

extern "C" void
UpdateAndRender(uint8 *BufferMemory, 
    uint16 ImageWidth, uint16 ImageHeight, 
    uint16 HouseWidth, uint16 HouseHeight,
    uint16 CarWidth, uint16 CarHeight,
    uint16 Width, uint16 Height, 
    uint16 MouseX, uint16 MouseY, bool MouseDown)
{
  if (!GameState.IsInitialized) 
  {
    GameState.VerticalCellCount = 8;
    GameState.HorizontalCellCount = 15;
    GameState.BackgroundColor = Color(0x21, 0x22, 0x31, 0xff);
    GameState.CellBorderColor = Color(0xcc, 0xcc, 0xcc, 0xff);
    GameState.CellBorderWidth = 1;
    GameState.CellSideInPixels = 64;

    GameState.BuildingBitmap = BufferMemory + (Width * Height * 4);
    GameState.HouseBitmap = GameState.BuildingBitmap + (ImageWidth * ImageHeight * 4);
    GameState.CarBitmap = GameState.HouseBitmap + (HouseWidth * HouseHeight * 4);

    GameState.Arena.Base = BufferMemory 
      + (Width * Height * 4)
      + (ImageWidth * ImageHeight * 4)
      + (CarWidth * CarHeight * 4);
    GameState.Arena.Used = 0;
    GameState.Arena.Size = 1024 * 1024 * 5; // 5MB


    GameState.MaxNumRoadNodes = 20;
    GameState.RoadNodes = PushArray(&GameState.Arena, GameState.MaxNumRoadNodes, road_node);
    road_node *Node = GameState.RoadNodes + 0;
    Node->XCellIndex = 2;
    Node->YCellIndex = 3;
    Node->Next[0] = GameState.RoadNodes + 1;
    Node->NumNeighbours = 1;
    Node->Visited = false;

    Node = GameState.RoadNodes + 1;
    Node->XCellIndex = 3;
    Node->YCellIndex = 3;
    Node->Next[0] = GameState.RoadNodes + 2;
    Node->NumNeighbours = 1;
    Node->Visited = false;

    Node = GameState.RoadNodes + 2;
    Node->XCellIndex = 4;
    Node->YCellIndex = 3;
    Node->NumNeighbours = 0;
    Node->Visited = false;

    GameState.MaxNumRoads = 5;
    GameState.UnConnectedRoads = PushArray(&GameState.Arena, GameState.MaxNumRoads, road_node *);
    GameState.UnConnectedRoads[0] = GameState.RoadNodes + 0;
    GameState.NumRoads = 1;

    GameState.MaxNumCars = 2;
    GameState.Cars = PushArray(&GameState.Arena, GameState.MaxNumCars, car);
    car *Car = GameState.Cars + 0;
    Car->Pixels = (pixel *)GameState.CarBitmap;
    Car->Width = CarWidth;
    Car->Height = CarHeight;

    GameState.MaxNumBuildings = 2;
    GameState.Buildings = PushArray(&GameState.Arena, GameState.MaxNumBuildings, building);
    building *Building = GameState.Buildings + 0;
    Building->Pixels = (pixel *)GameState.BuildingBitmap;
    Building->Width = ImageWidth;
    Building->Height = ImageHeight;
    Building->CellIndexX = 1;
    Building->CellIndexY = 1;

    GameState.MaxNumHouses = 5;
    GameState.Houses = PushArray(&GameState.Arena, GameState.MaxNumHouses, house);
    house *House = GameState.Houses + 0;
    House->Pixels = (pixel *)GameState.HouseBitmap;
    House->Width = HouseWidth;
    House->Height = HouseHeight;
    House->CellIndexX = 5;
    House->CellIndexY = 5;

    House++;
    House->Pixels = (pixel *)GameState.HouseBitmap;
    House->Width = HouseWidth;
    House->Height = HouseHeight;
    House->CellIndexX = 5;
    House->CellIndexY = 6;

    GameState.Buffer.Pixels = (pixel *)BufferMemory;
    GameState.Buffer.Width = Width;
    GameState.Buffer.Height = Height;

    GameState.IsInitialized = true;
  }

  FillColor(&GameState, 0, 0, Width, Height, 
      GameState.BackgroundColor);

  if (MouseDown)
  {
    DrawGrid(&GameState);
  }

  DrawRoads(&GameState);

  DrawBuildings(&GameState);

  DrawHouses(&GameState);

  uint8 *CarBuffer = BufferMemory + (Width * Height * 4) + (ImageWidth * ImageHeight * 4) + (HouseWidth * HouseHeight * 4);
  DrawImage(&GameState, (pixel *)CarBuffer, CarWidth, CarHeight, Width, 5, 4, 20, 20);

  if (MouseDown)
  {
    uint16 StartX = GameState.MouseDownCellIndexX * GameState.CellSideInPixels + 0.5f * GameState.CellSideInPixels;
    uint16 StartY = GameState.MouseDownCellIndexY * GameState.CellSideInPixels + 0.5f * GameState.CellSideInPixels;
    uint16 EndX = MouseX;
    uint16 EndY = MouseY;

    if (!GameState.MouseIsDown) 
    {
      GameState.MouseIsDown = true;
      GameState.MouseDownCellIndexX = MouseX / GameState.CellSideInPixels;
      GameState.MouseDownCellIndexY = MouseY / GameState.CellSideInPixels;
    }
    else 
    {
      DrawFilledCircle(&GameState, 
        EndX,
        EndY,
        20, GameState.CellBorderColor);

      uint8 CurrentCellIndexX = EndX / GameState.CellSideInPixels;
      uint8 CurrentCellIndexY = EndY / GameState.CellSideInPixels;


      DrawLine(&GameState, StartX, StartY, EndX, EndY, GameState.CellBorderColor);

      uint16 SegmentLength2 = Square((GameState.MouseDownCellIndexX - CurrentCellIndexX) * GameState.CellSideInPixels) + 
        Square((GameState.MouseDownCellIndexY - CurrentCellIndexY) * GameState.CellSideInPixels);
      if (SegmentLength2 >= Square(GameState.CellSideInPixels))
      {
        CreateAndInsertNodes(&GameState, 
            GameState.MouseDownCellIndexX, GameState.MouseDownCellIndexY,
            CurrentCellIndexX, CurrentCellIndexY);

        GameState.MouseDownCellIndexX = CurrentCellIndexX;
        GameState.MouseDownCellIndexY = CurrentCellIndexY;
      }
      else
      {
        DrawLine(&GameState, StartX, StartY, EndX, EndY, GameState.CellBorderColor);
      }
    }

    DrawFilledCircle(&GameState, 
      StartX,
      StartY,
      20, GameState.CellBorderColor);
  }
  else 
  {
    if (GameState.MouseIsDown)
    {
      GameState.MouseIsDown = false;
    }
  }
  // DrawCircle(BufferMemory, Width, Height, MouseX, MouseY, 20, CellBorderColor, 4);
}
