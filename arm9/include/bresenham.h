#ifndef BRESENHAM_H
#define BRESENHAM_H

#include <nds.h>

#ifndef abs
#define abs(x) ((x) < 0 ? -(x) : (x))
#endif

void setpx8(u8 x, u8 y, u8 c) {
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
}

void bresenTrace(u8* buf, u8 x1, u8 y1, u8 x2, u8 y2, u16 c)
{
	int dx, dy, inx, iny, e;
	
	dx = x2 - x1;
	dy = y2 - y1;
	inx = dx > 0 ? 1 : -1;
	iny = dy > 0 ? 1 : -1;

	dx = abs(dx);
	dy = abs(dy);
	
	if(dx >= dy) {
		dy <<= 1;
		e = dy - dx;
		dx <<= 1;
		while (x1 != x2) {
			//setpx8(x1,y1,c);
      buf[x1+y1*SCREEN_WIDTH] = c;
			//BG_GFX[y1*SCREEN_WIDTH + x1] = c;
			if(e >= 0) {
				y1 += iny;
				e-= dx;
			}
			e += dy; x1 += inx;
		}
	} else {
		dx <<= 1;
		e = dx - dy;
		dy <<= 1;
		while (y1 != y2) {
      buf[x1+y1*SCREEN_WIDTH] = c;
			//setpx8(x1,y1,c);
			//BG_GFX[y1*SCREEN_WIDTH + x1] = c;
			if(e >= 0) {
				x1 += inx;
				e -= dy;
			}
			e += dx; y1 += iny;
		}
	}
	//setpx8(x1,y1,c);
  buf[x1+y1*SCREEN_WIDTH] = c;
	//BG_GFX[y1*SCREEN_WIDTH + x1] = c;
}

#endif
