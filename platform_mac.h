#if !defined(PLATFORM_MAC_H)
#define PLATFORM_MAC_H

typedef unsigned char uint8 ;
typedef unsigned short uint16;
typedef unsigned long uint32;

typedef signed char int8;
typedef signed short int16;
typedef signed long int32;

union pixel 
{
  struct 
  {
    uint8 R;
    uint8 G;
    uint8 B;
    uint8 A;
  };

  uint32 Color;
};

#endif
