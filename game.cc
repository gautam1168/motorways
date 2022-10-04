struct pixel
{
  char R;
  char G;
  char B;
  char A;
};

extern "C" void
UpdateAndRender(char *BufferMemory, int Width, int Height)
{
  pixel *Pixels = (pixel *)BufferMemory;
  for (int PixelIndex = 0;
      PixelIndex < Width * Height;
      ++PixelIndex
      )
  {
    pixel *Pixel = Pixels + PixelIndex;
    Pixel->R = 0xff;
    Pixel->G = 0x00;
    Pixel->B = 0x00;
    Pixel->A = 0xff;
  }
}
