#include <bresenham.h>
#include <string.h>

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
      buf[x1+y1*256] = c;
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
      buf[x1+y1*256] = c;
      if(e >= 0) {
        x1 += inx;
        e -= dy;
      }
      e += dx; y1 += iny;
    }
  }
  buf[x1+y1*256] = c;
}

// x,y values are all 32:0, width is 16:16
void bresenThick(u8* buf, int x1, int y1, int x2, int y2, u8 val, int width) {
  int dx, dy,  x2_, y2_;
  dx = x2 - x1;
  dy = y2 - y1;

  if (dx == 0 && dy == 0) { buf[y1*256+x1] = val; return; }

  if (abs(dx) > abs(dy)) { // step along X axis
    if (dx < 0) { // swap!
      x1 ^= x2; x2 ^= x1; x1 ^= x2;
      y1 ^= y2; y2 ^= y1; y1 ^= y2;
      dx = -dx; dy = -dy;
    }
    y1 = (y1 << 16) - (width>>1) + 0x8000;
    dy = (dy<<16)/dx;
    y1 += dy>>1;
    if (x1 < 0) { y1 += dy * -x1; x1 = 0; }
    if (x2 > 255) x2 = 255;
    for (dx = x1; dx <= x2; dx++) {
      y2 = (y1 >> 16);
      if (y2 < 0) y2 = 0; // clip
      y2_ = (y1+width) >> 16;
      if (y2_ > 192) y2_ = 192;

      for (; y2 < y2_; y2++)
        buf[y2*256+dx] = val;
      y1 += dy;
    }

  } else { // step along Y axis
    if (dy < 0) { // swap!
      x1 ^= x2; x2 ^= x1; x1 ^= x2;
      y1 ^= y2; y2 ^= y1; y1 ^= y2;
      dx = -dx; dy = -dy;
    }
    x1 = (x1 << 16) - (width>>1) + 0x8000;
    dx = (dx<<16)/dy;
    x1 += dx>>1;
    if (y1 < 0) { x1 += dx * -y1; y1 = 0; }
    if (y2 > 191) y2 = 191;
    for (dy = y1; dy <= y2; dy++) {
      x2 = (x1>>16);
      if (x2 < 0) x2 = 0; // clip
      x2_ = (x1+width) >> 16;
      if (x2_ > 255) x2_ = 255;

      for (; x2 < x2_; x2++)
        buf[dy*256+x2] = val;
      x1 += dx;
    }
  }
}


void bresenCircle(u8* buf, s32 cx, s32 cy, s32 r, u8 val) {
  s32 x = r, y = 1;
  s32 xchg = 1 - 2*r, ychg = 3;
  s32 raderr = 1;
  while (x >= y) {
    memset(buf+(cx-x)+256*(cy+y-1), val, 2*x);
    memset(buf+(cx-x)+256*(cy-y), val, 2*x);
    memset(buf+(cx-y)+256*(cy+x-1), val, 2*y);
    memset(buf+(cx-y)+256*(cy-x), val, 2*y);
    y++;
    raderr += ychg;
    ychg += 2;
    if (2 * raderr + xchg > 0) {
      x--;
      raderr += xchg;
      xchg += 2;
    }
  }
}

/*void bresenCircle(u8* buf, s32 x, s32 y, s32 r, u8 val) {
  s32 cx = 0;
  s32 cy = r;
  s32 ocx = 0xffff;
  s32 ocy = 0xffff;
  s32 df = 1 - r;
  s32 d_e = 3;
  s32 d_se = -2 * r + 5;
  s32 xpcx, xmcx, xpcy, xmcy;
  s32 ypcy, ymcy, ypcx, ymcx;

  if (r == 0) { buf[cx+256*cy] = val; return; }

  do {
    xpcx = x + cx;
    xmcx = x - cx;
    xpcy = x + cy;
    xmcy = x - cy;
    if (ocy != cy) {
      if (cy > 0) {
        ypcy = y + cy;
        ymcy = y - cy;
        memset(buf+xmcx+ypcy*256, val, xpcx - xmcx);
        memset(buf+xmcx+ymcy*256, val, xpcx - xmcx);
      } else {
        memset(buf+xmcx+y*256, val, xpcx - xmcx);
      }
      ocy = cy;
    }
    if (ocx != cx) {
      if (cx != cy) {
        if (cx > 0) {
          ypcx = y + cx;
          ymcx = y - cx;
          memset(buf+xmcy+ymcx*256, val, xpcy - xmcy);
          memset(buf+xmcy+ypcx*256, val, xpcy - xmcy);
        } else {
          memset(buf+xmcy+y*256, val, xpcy - xmcy);
        }
      }
      ocx = cx;
    }
    if (df < 0) {
      df += d_e;
      d_e += 2;
      d_se += 2;
    } else {
      df += d_se;
      d_e += 2;
      d_se += 4;
      cy--;
    }
    cx++;
  } while (cx <= cy);
}*/
