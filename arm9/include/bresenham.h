#ifndef BRESENHAM_H
#define BRESENHAM_H

#include <nds.h>

#ifndef abs
#define abs(x) ((x) < 0 ? -(x) : (x))
#endif

/*void setpx8(u8 x, u8 y, u8 c) {
	// this is where in the memory the pixel would be ... if it were 16bpp
	u16 pos = y*SCREEN_WIDTH+x;
	// divide by two to get the 'real' location of the pixel
	u16 px16 = BG_GFX[pos/2];
	// if pos is even, we want to change the left pixel and keep the right.
	// if pos is odd, we want to change the right pixel and keep the left.
	// here's some scary bitwise operations that do the job.
	BG_GFX[pos/2] = (pos % 2) ?
				((u16)(px16 & 0xff) | (u16)c << 8) :
				((u16)((px16 >> 8) << 8) | (u16)c);
}

u8 getpx8(u8 x, u8 y) {
	u16 pos = y*SCREEN_WIDTH+x;
	u16 px16 = BG_GFX[pos/2];
	// same deal; & 0xff gives us the right pixel, >> 8 gives us the left
	// note that (as far as I undestand) if the u16 is, say, 0xabcd, 0xcd
	// actually represents the left pixel, while 0xab is the right. Good
	// question.
	return (u8)((pos % 2) ? (px16 & 0xff) : (px16 >> 8));
}*/

void bresenTrace(u8* buf, u8 x1, u8 y1, u8 x2, u8 y2, u16 c);
void bresenThick(u8* buf, int x1, int y1, int x2, int y2, u8 val, int width);
void bresenCircle(u8* buf, s32 cx, s32 cy, s32 r, u8 val);

#endif
