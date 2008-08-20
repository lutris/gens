//#include <windows.h>
#include "blit.h"

#ifdef __cplusplus
extern "C"
{
  
#endif	/*  */
  
//void hq2x_16(unsigned char*, unsigned char*, int, int, int, int);
  unsigned int _LUT16to32[65536];
   unsigned int _RGBtoYUV[65536];
   
//extern unsigned char MD_Screen;
#include "vdp_rend.h"
#include "misc.h"
#ifdef __cplusplus
} 
#endif	/*  */
 int
Blit_HQ2x_InitLUTs (void) 
{
  int i, j, k, r, g, b, Y, u, v;
  for (i = 0; i < 65536; i++)
    _LUT16to32[i] =
      ((i & 0xF800) << 8) + ((i & 0x07E0) << 5) + ((i & 0x001F) << 3);
  for (i = 0; i < 32; i++)
    for (j = 0; j < 64; j++)
      for (k = 0; k < 32; k++)
	
	{
	  r = i << 3;
	  g = j << 2;
	  b = k << 3;
	  Y = (r + g + b) >> 2;
	  u = 128 + ((r - b) >> 2);
	  v = 128 + ((-r + 2 * g - b) >> 3);
	  _RGBtoYUV[(i << 11) + (j << 5) + k] = (Y << 16) + (u << 8) + v;
	}
  
/*  __asm
  {
    mov  eax, 1
    cpuid
    and  edx, 0x00800000
    mov  nMMXsupport, edx
  }

  return nMMXsupport;
  */ 
    return Have_MMX;
}


//void Blit_HQ2x(unsigned char *Dest, int pitch, int x, int y, int offset) {
  // pitch = bytes between dest rows (1280)
  // x = source width /pixels (320) (256)
  // y = source height /pixels (224)
  // offset = bytes between lines (32) (160)
  // x + offset/2 == 336
  
/*	offset = 32 + Dep * 2
	x = 320 - Dep
	Dep = 320 - x
	offset = 32 + (320 - x) * 2
	x + offset/2 = 16 + 320 = 336

	Dep = 320 - x*/ 
//      pitch+=(320-x)*4;
  
//      char b[100];
//      sprintf(b,"pitch=%d x=%d y=%d offset=%d",pitch,x,y,offset);
//      MessageBox(0,b,NULL,0);
  
//  hq2x_16( MD_Screen+16, Dest, x, y, pitch, offset);
//  return;
//}
