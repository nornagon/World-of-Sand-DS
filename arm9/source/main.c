/*---------------------------------------------------------------------------------
	$Id: template.c,v 1.4 2005/09/17 23:15:13 wntrmute Exp $

	Basic Hello World

	$Log: template.c,v $
	Revision 1.4  2005/09/17 23:15:13  wntrmute
	corrected iprintAt in templates
	
	Revision 1.3  2005/09/05 00:32:20  wntrmute
	removed references to IPC struct
	replaced with API functions
	
	Revision 1.2  2005/08/31 01:24:21  wntrmute
	updated for new stdio support

	Revision 1.1  2005/08/03 06:29:56  wntrmute
	added templates


---------------------------------------------------------------------------------*/
#include <nds.h>
#include <nds/arm9/console.h> //basic print funcionality
#include <nds/arm9/trig_lut.h>
#include <gbfs.h>

#include "mem.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "bresenham.h"
#include "mt19937ar.h" // mersenne twister

#include "brushes_bin.h"
#include "brushes_pal_bin.h"
#include "selector_bin.h"
#include "selector_pal_bin.h"
#include "map_bin.h"

#define CHANCE(n) (genrand_int32() < ((u32)((n)*0xffffffff)))

typedef enum {
  NOTHING = 0,
  SAND,
  WATER,
  FIRE,
  WALL,
  PLANT,
  SMOKE,
  ASH,
  SPOUT,
  CERA,
  CERA2,
  UNID,
  UNIDT,
  OIL,
  SWATER,
  SALT,
  SNOW,
  STEAM,
  CONDEN,
  NUM_MATERIALS
} MATERIAL;

bool LIQUID[NUM_MATERIALS] = {
  false, // NOTHING
  false, // SAND
  true, // WATER
  false, // FIRE
  false, // WALL
  false, // PLANT
  false, // SMOKE
  false, // ASH
  false, // SPOUT
  false, // CERA
  false, // CERA2
  false, // UNID
  false, // UNIDT
  true, // OIL
  true, // SWATER
  false, // SALT
  false, // SNOW
  false, // STEAM
  false, // CONDEN
};

static inline void addsome(MATERIAL type, u8* buf, u16 x) {
//  int i,j;
//  for (i = 1; i <= 4; i++)
//    for (j = -4; j <= 4; j++)
//      if (CHANCE(0.1))
//        buf[i*256+x+j] = type;
// unroll'd!
  if (CHANCE(0.1)) buf[1*256+x-4] = type;
  if (CHANCE(0.1)) buf[1*256+x-3] = type;
  if (CHANCE(0.1)) buf[1*256+x-2] = type;
  if (CHANCE(0.1)) buf[1*256+x-1] = type;
  if (CHANCE(0.1)) buf[1*256+x-0] = type;
  if (CHANCE(0.1)) buf[1*256+x+1] = type;
  if (CHANCE(0.1)) buf[1*256+x+2] = type;
  if (CHANCE(0.1)) buf[1*256+x+3] = type;
  if (CHANCE(0.1)) buf[1*256+x+4] = type;
  if (CHANCE(0.1)) buf[2*256+x-4] = type;
  if (CHANCE(0.1)) buf[2*256+x-3] = type;
  if (CHANCE(0.1)) buf[2*256+x-2] = type;
  if (CHANCE(0.1)) buf[2*256+x-1] = type;
  if (CHANCE(0.1)) buf[2*256+x-0] = type;
  if (CHANCE(0.1)) buf[2*256+x+1] = type;
  if (CHANCE(0.1)) buf[2*256+x+2] = type;
  if (CHANCE(0.1)) buf[2*256+x+3] = type;
  if (CHANCE(0.1)) buf[2*256+x+4] = type;
  if (CHANCE(0.1)) buf[3*256+x-4] = type;
  if (CHANCE(0.1)) buf[3*256+x-3] = type;
  if (CHANCE(0.1)) buf[3*256+x-2] = type;
  if (CHANCE(0.1)) buf[3*256+x-1] = type;
  if (CHANCE(0.1)) buf[3*256+x-0] = type;
  if (CHANCE(0.1)) buf[3*256+x+1] = type;
  if (CHANCE(0.1)) buf[3*256+x+2] = type;
  if (CHANCE(0.1)) buf[3*256+x+3] = type;
  if (CHANCE(0.1)) buf[3*256+x+4] = type;
  if (CHANCE(0.1)) buf[4*256+x-4] = type;
  if (CHANCE(0.1)) buf[4*256+x-3] = type;
  if (CHANCE(0.1)) buf[4*256+x-2] = type;
  if (CHANCE(0.1)) buf[4*256+x-1] = type;
  if (CHANCE(0.1)) buf[4*256+x-0] = type;
  if (CHANCE(0.1)) buf[4*256+x+1] = type;
  if (CHANCE(0.1)) buf[4*256+x+2] = type;
  if (CHANCE(0.1)) buf[4*256+x+3] = type;
  if (CHANCE(0.1)) buf[4*256+x+4] = type;
}

static inline void spawn(u8* buf) {
  addsome(SAND, buf, 51);
  addsome(WATER, buf, 102);
  addsome(SALT, buf, 153);
  addsome(OIL, buf, 204);
}

static void majic(u8* buf, u32 x, u32 y) {
  u8* top = buf+(x-1)+(y-1)*256,
    * mid = buf+(x-1)+(y)*256,
    * bot = buf+(x-1)+(y+1)*256;
  // ttt
  // mxm
  // bbb
  // px = mid[1]
  switch(mid[1]) {
    case SAND:
      if (CHANCE(0.95)) {
        // gravity
        if (bot[1] == NOTHING) { mid[1] = NOTHING; bot[1] = SAND; break; }
        if (bot[0] == NOTHING) { mid[1] = NOTHING; bot[0] = SAND; break; }
        if (bot[2] == NOTHING) { mid[1] = NOTHING; bot[2] = SAND; break; }
        if (mid[0] == NOTHING) { mid[1] = NOTHING; mid[0] = SAND; break; }
        if (mid[2] == NOTHING) { mid[1] = NOTHING; mid[2] = SAND; break; }
      } else
      if (CHANCE(0.25)) {
        // sink below water
        if (bot[1] == WATER) { bot[1] = SAND; mid[1] = WATER; break; }
        if (bot[0] == WATER) { bot[0] = SAND; mid[1] = WATER; break; }
        if (bot[2] == WATER) { bot[2] = SAND; mid[1] = WATER; break; }
      }
      break;
    case WATER:
      if (CHANCE(0.95)) {
        // gravity
        if (bot[1] == NOTHING) { bot[1] = WATER; mid[1] = NOTHING; break; }
        if (bot[0] == NOTHING) { bot[0] = WATER; mid[1] = NOTHING; break; }
        if (bot[2] == NOTHING) { bot[2] = WATER; mid[1] = NOTHING; break; }
        if (mid[2] == NOTHING) { mid[2] = WATER; mid[1] = NOTHING; break; }
        if (mid[0] == NOTHING) { mid[0] = WATER; mid[1] = NOTHING; break; }
      }
      break;
    case SWATER:
      if (CHANCE(0.95)) {
        // gravity
        if (bot[1] == NOTHING) { bot[1] = SWATER; mid[1] = NOTHING; break; }
        if (bot[0] == NOTHING) { bot[0] = SWATER; mid[1] = NOTHING; break; }
        if (bot[2] == NOTHING) { bot[2] = SWATER; mid[1] = NOTHING; break; }
        if (mid[2] == NOTHING) { mid[2] = SWATER; mid[1] = NOTHING; break; }
        if (mid[0] == NOTHING) { mid[0] = SWATER; mid[1] = NOTHING; break; }
      }
      if (bot[1] == WATER && CHANCE(0.5)) {
        // sink below water
        bot[1] = SWATER;
        mid[1] = WATER;
        break;
      }
      if (CHANCE(0.3)) {
        if (bot[0] == WATER) { bot[0] = SWATER; mid[1] = WATER; break; }
        if (bot[2] == WATER) { bot[2] = SWATER; mid[1] = WATER; break; }
        if (mid[2] == WATER) { mid[2] = SWATER; mid[1] = WATER; break; }
        if (mid[0] == WATER) { mid[0] = SWATER; mid[1] = WATER; break; }
      }
      break;
    case UNID:
      if (CHANCE(0.65)) {
        if (bot[1] != UNIDT && bot[1] != PLANT) bot[1] = UNID;
        if (top[1] != UNIDT && top[1] != PLANT) top[1] = UNID;
        if (mid[0] != UNIDT && mid[0] != PLANT) mid[0] = UNID;
        if (mid[2] != UNIDT && mid[2] != PLANT) mid[2] = UNID;
      } else
        mid[1] = UNIDT;
      break;
    case UNIDT:
      if (CHANCE(0.02)) mid[1] = NOTHING;
      break;
    case CERA:
      if (CHANCE(0.99)) break;
      if (bot[1] == FIRE ||
          mid[0] == FIRE ||
          mid[2] == FIRE ||
          top[1] == FIRE) {
        mid[1] = FIRE;
             if (bot[1] == NOTHING) bot[1] = CERA2;
        else if (mid[2] == NOTHING) mid[2] = CERA2;
        else if (mid[0] == NOTHING) mid[0] = CERA2;
        else if (top[1] == NOTHING) top[1] = CERA2;
      }
      break;
    case CERA2:
      if (CHANCE(0.2)) break;
      // slide stickily
           if (bot[1] == NOTHING) bot[1] = CERA2, mid[1] = NOTHING;
      else if (bot[0] == NOTHING) bot[0] = CERA2, mid[1] = NOTHING;
      else if (bot[2] == NOTHING) bot[2] = CERA2, mid[1] = NOTHING;
      else mid[1] = CERA; // dry
      break;
    case FIRE:
      if (CHANCE(0.5)) {
        // rand between 0 and 2, weighted double for 1
        //u8 n = (rand() > RAND_MAX/4 ? (rand() < RAND_MAX*0.75 ? 1 : 2) : 0);
        if (top[1] == NOTHING) top[1] = FIRE;
      }
      if (CHANCE(0.4) &&
        // die if no nearby flammables
        top[1] != PLANT &&
        mid[0] != PLANT &&
        mid[2] != PLANT &&
        bot[1] != PLANT &&
        top[1] != CERA &&
        mid[0] != CERA &&
        mid[2] != CERA &&
        bot[1] != CERA)
        mid[1] = NOTHING;
      if (CHANCE(0.9)) {
        // water + fire = steam
        if (mid[2] == WATER) { mid[2] = STEAM; mid[1] = NOTHING; break; }
        if (mid[0] == WATER) { mid[0] = STEAM; mid[1] = NOTHING; break; }
        if (bot[0] == WATER) { bot[0] = STEAM; mid[1] = NOTHING; break; }
      }
      break;
    case PLANT:
      if (CHANCE(0.2)) {
        // burn
        if (bot[1] == FIRE) {
          if (CHANCE(0.2)) { bot[1] = SMOKE; } mid[1] = FIRE; break; }
        if (mid[0] == FIRE) {
          if (CHANCE(0.2)) { mid[0] = SMOKE; } mid[1] = FIRE; break; }
        if (mid[2] == FIRE) {
          if (CHANCE(0.2)) { mid[2] = SMOKE; } mid[1] = FIRE; break; }
        if (top[1] == FIRE) {
          if (CHANCE(0.2)) { top[1] = SMOKE; } mid[1] = FIRE; break; }
      }
      if (CHANCE(0.1) &&
          (top[1] == SWATER ||
           mid[0] == SWATER ||
           mid[2] == SWATER ||
           bot[1] == SWATER)) {
        mid[1] = NOTHING; // salt kills
        break;
      }

      // grooow, slurp
      if (CHANCE(0.1) && mid[0] == WATER) mid[0] = PLANT;
      if (CHANCE(0.1) && mid[2] == WATER) mid[2] = PLANT;
      if (CHANCE(0.1) && top[1] == WATER) top[1] = PLANT;
      if (CHANCE(0.1) && bot[1] == WATER) bot[1] = PLANT;
      break;
    case SMOKE:
      if (CHANCE(0.4)) {
        if (CHANCE(0.6)) {
          if (CHANCE(0.5)) {
            if (top[0] == NOTHING) { top[0] = SMOKE; mid[1] = NOTHING; break; }
            if (top[2] == NOTHING) { top[2] = SMOKE; mid[1] = NOTHING; break; }
          } else {
            if (top[2] == NOTHING) { top[2] = SMOKE; mid[1] = NOTHING; break; }
            if (top[0] == NOTHING) { top[0] = SMOKE; mid[1] = NOTHING; break; }
          }
        } else
          if (top[1] == NOTHING) { top[1] = SMOKE; mid[1] = NOTHING; break; }
      }
      if (CHANCE(0.01)) { mid[1] = ASH; break; }
      break;
    case ASH:
      if (CHANCE(0.8)) {
        if (CHANCE(0.5)) {
          if (CHANCE(0.5)) {
            if (bot[0] == NOTHING || LIQUID[bot[0]])
              { mid[1] = bot[0]; bot[0] = ASH; break; }
            if (bot[2] == NOTHING || LIQUID[bot[2]])
              { mid[1] = bot[2]; bot[2] = ASH; break; }
          } else {
            if (bot[2] == NOTHING || LIQUID[bot[2]])
              { mid[1] = bot[2]; bot[2] = ASH; break; }
            if (bot[0] == NOTHING || LIQUID[bot[0]])
              { mid[1] = bot[0]; bot[0] = ASH; break; }
          }
        } else {
          if (bot[1] == NOTHING || LIQUID[bot[1]])
            { mid[1] = bot[1]; bot[1] = ASH; break; }
        }
      }
      break;
    case SPOUT:
      if (CHANCE(0.05)) {
        if (top[1] == NOTHING) top[1] = WATER;
        if (bot[1] == NOTHING) bot[1] = WATER;
        if (mid[0] == NOTHING) mid[0] = WATER;
        if (mid[2] == NOTHING) mid[2] = WATER;
      }
      if (CHANCE(0.9)) break;

      // sand destroys spout
      if (top[1] == SAND) mid[1] = SAND;
      if (mid[0] == SAND) mid[1] = SAND;
      if (mid[2] == SAND) mid[1] = SAND;
      if (bot[1] == SAND) mid[1] = SAND;
      break;
    case OIL:
      if (CHANCE(0.25))
        if (bot[1] == FIRE ||
            mid[0] == FIRE ||
            mid[2] == FIRE ||
            top[1] == FIRE) {
          mid[0] = mid[1] = mid[2] = top[1] = bot[1] = FIRE; // boom! :)
          break;
        }
      if (CHANCE(0.95)) {
        // gravity
             if (bot[1] == NOTHING) { bot[1] = OIL; mid[1] = NOTHING; break; }
        else if (bot[0] == NOTHING) { bot[0] = OIL; mid[1] = NOTHING; break; }
        else if (bot[2] == NOTHING) { bot[2] = OIL; mid[1] = NOTHING; break; }
        else if (mid[2] == NOTHING) { mid[2] = OIL; mid[1] = NOTHING; break; }
        else if (mid[0] == NOTHING) { mid[0] = OIL; mid[1] = NOTHING; break; }
      }
      if (CHANCE(0.95)) {
             if (top[1] == WATER) { top[1] = OIL; mid[1] = WATER; break; }
        else if (top[0] == WATER) { top[0] = OIL; mid[1] = WATER; break; }
        else if (top[2] == WATER) { top[2] = OIL; mid[1] = WATER; break; }
      }
      if (CHANCE(0.3)) {
             if (mid[2] == WATER) { mid[2] = OIL; mid[1] = WATER; break; }
        else if (mid[0] == WATER) { mid[0] = OIL; mid[1] = WATER; break; }
      }
      break;
    case SALT:
      if (CHANCE(0.95)) {
        // gravity
             if (bot[1] == NOTHING) { bot[1] = SALT; mid[1] = NOTHING; break; }
        else if (bot[0] == NOTHING) { bot[0] = SALT; mid[1] = NOTHING; break; }
        else if (bot[2] == NOTHING) { bot[2] = SALT; mid[1] = NOTHING; break; }
        else if (mid[2] == NOTHING) { mid[2] = SALT; mid[1] = NOTHING; break; }
        else if (mid[0] == NOTHING) { mid[0] = SALT; mid[1] = NOTHING; break; }
      }
      if (CHANCE(0.9)) {
             if (bot[1] == WATER) bot[1] = SWATER;
        else if (top[1] == WATER) top[1] = SWATER;
        else if (mid[0] == WATER) mid[0] = SWATER;
        else if (mid[2] == WATER) mid[2] = SWATER;
        else break;
        mid[1] = NOTHING;
      }
      break;
    case SNOW:
      if ((bot[1] == FIRE || mid[0] == FIRE || mid[2] == FIRE || top[1] == FIRE)
          && CHANCE(0.95)) {
        mid[1] = WATER; break;
      }
      if (CHANCE(0.95)) {
        // gravity
        if (CHANCE(0.5)) {
               if (bot[0] == NOTHING) { bot[0] = SNOW; mid[1] = NOTHING; break; }
          else if (bot[2] == NOTHING) { bot[2] = SNOW; mid[1] = NOTHING; break; }
        } else {
               if (bot[2] == NOTHING) { bot[2] = SNOW; mid[1] = NOTHING; break; }
          else if (bot[0] == NOTHING) { bot[0] = SNOW; mid[1] = NOTHING; break; }
        }
        if (bot[1] == NOTHING) { bot[1] = SNOW; mid[1] = NOTHING; break; }
      }
      if (CHANCE(0.01)) {
        // melt in air
        if (mid[0] == NOTHING || mid[2] == NOTHING ||
            top[1] == NOTHING || bot[1] == NOTHING) { mid[1] = WATER; break; }
        // turn water into snow
        if (CHANCE(0.2)) {
          if (mid[0] == WATER) { mid[0] = SNOW; break; }
          if (mid[2] == WATER) { mid[2] = SNOW; break; }
          if (bot[1] == WATER) { bot[1] = SNOW; break; }
          if (top[1] == WATER) { top[1] = SNOW; break; }
        }
      }
      if (CHANCE(0.8)) {
        if (top[1] == SALT) { mid[1] = SWATER; top[1] = NOTHING; break; }
        if (mid[0] == SALT) { mid[1] = SWATER; mid[0] = NOTHING; break; }
        if (mid[2] == SALT) { mid[1] = SWATER; mid[2] = NOTHING; break; }
        if (bot[1] == SALT) { mid[1] = SWATER; bot[1] = NOTHING; break; }
      }
      break;
    case STEAM:
      if (CHANCE(0.5)) {
        if (top[1] == NOTHING || LIQUID[top[1]]) {
          mid[1] = top[1]; top[1] = STEAM; break; }
        if (top[0] == NOTHING || LIQUID[top[0]]) {
          mid[1] = top[0]; top[0] = STEAM; break; }
        if (top[2] == NOTHING || LIQUID[top[2]]) {
          mid[1] = top[2]; top[2] = STEAM; break; }
        if (mid[0] == NOTHING) { mid[0] = STEAM; mid[1] = NOTHING; break; }
        if (mid[2] == NOTHING) { mid[2] = STEAM; mid[1] = NOTHING; break; }
      } else if (CHANCE(0.2) &&
          (top[1] == WALL || top[1] == CERA || top[1] == PLANT)) {
        mid[1] = CONDEN;
      }
      break;
    case CONDEN:
      if (CHANCE(0.01) ||
          (top[1] != WALL && top[1] != PLANT && top[1] != CERA)) {
        mid[1] = WATER; break; }
      break;
  }
}

void calculate(u8* buf) {
  static int counter = 0;
  int x,y;

  if (counter)
    for (y = 191; y > 0; y--) // ^
      for (x = 1; x < 255; x++) // ->
        majic(buf,x,y);
  else
    for (y = 191; y > 0; y--) // ^
      for (x = 255; x > 0; x--) // <-
        majic(buf,x,y);
  // drop a black rectangle over it all
  memset32(buf, NOTHING, 64);
  memset32(buf+192*256, NOTHING, 64);
  for (y = 1; y < 192; y++)
    buf[y*256] = buf[y*256+255] = NOTHING;
  
  counter = !counter; // go the other way next time
}

u32 __seed_val = 0xfeebdaed;

inline void reseed() { __seed_val ^= IPC->rtc_seconds; }
inline u32 myrand() {
  return (__seed_val = (0xac458abe * __seed_val + 0x7dd8915c) % 0x55610be1);
}

inline void initOAM(SpriteRotation* rot) {
  u8 i;
  for (i = 0; i < 32; ++i) {
    rot[i].filler1[0] = rot[i].filler2[0] =
      rot[i].filler3[0] = rot[i].filler4[0] = ATTR0_DISABLED;
    rot[i].filler1[1] = rot[i].filler1[2] =
      rot[i].filler2[1] = rot[i].filler2[2] =
      rot[i].filler3[1] = rot[i].filler3[2] =
      rot[i].filler4[1] = rot[i].filler4[2] = 0;
    rot[i].hdx = 256;
    rot[i].hdy = 0;
    rot[i].vdx = 0;
    rot[i].vdy = 256;
  }
}

inline void moveSprite(SpriteEntry *spr, u16 x, u16 y) {
  spr->attribute[1] &= 0xfe00;
  spr->attribute[1] |= (x & 0x01ff);
  spr->attribute[0] &= 0xff00;
  spr->attribute[0] |= (y & 0x00ff);
}

inline void rotSprite(SpriteRotation *rot, u16 angle) {
  s16 s = SIN[angle & 0x1ff] >> 4;
  s16 c = COS[angle & 0x1ff] >> 4;
  rot->hdx = c;
  rot->hdy = s;
  rot->vdx = -s;
  rot->vdy = c;
}

const u16 arrow_pal_rot[] = {
  RGB15(9,9,9),
  RGB15(13,13,13),
  RGB15(21,21,21),
  RGB15(25,25,25),
  RGB15(31,31,31),
  RGB15(25,25,25),
  RGB15(21,21,21),
  RGB15(13,13,13),
  RGB15(9,9,9),
  RGB15(13,13,13),
};

SpriteEntry oam_back[128];
SpriteRotation *const oam_back_rot = (SpriteRotation*)oam_back;

//---------------------------------------------------------------------------------
int main(void) {
//---------------------------------------------------------------------------------
	touchPosition touch;
  u8 *buf = malloc(256*192);
  memset(buf, 0, 256*192);

  irqInit();
  irqEnable(IRQ_VBLANK);

  /********
   * GBFS *
   ********/

  //sysSetCartOwner(BUS_OWNER_ARM9);
  //GBFS_FILE const* gbfs_file = find_first_gbfs_file((void*)0x08000000);

  /************************
   * Video Initialization *
   ************************/

  lcdSwap(); // main on bottom screen

  // --**ooOO- Main BG -OOoo**--

	videoSetMode(MODE_5_2D | DISPLAY_BG3_ACTIVE);

	vramSetBankA(VRAM_A_MAIN_BG);

  BG3_CR = BG_BMP8_256x256 | BG_PRIORITY(2);
  BG3_XDX = 1 << 8;
  BG3_XDY = 0;
  BG3_YDX = 0;
  BG3_YDY = 1 << 8;

  BG_PALETTE[NOTHING] = RGB15(0,0,0);
  BG_PALETTE[SAND] = RGB15(29,25,16);
  BG_PALETTE[WATER] = RGB15(4,4,31);
  BG_PALETTE[FIRE] = RGB15(31,8,8);
  BG_PALETTE[WALL] = RGB15(16,16,16);
  BG_PALETTE[PLANT] = RGB15(4,25,4);
  BG_PALETTE[SMOKE] = RGB15(10,10,10);
  BG_PALETTE[ASH] = RGB15(10,10,10);
  BG_PALETTE[SPOUT] = RGB15(14,20,31);
  BG_PALETTE[CERA] = RGB15(29,27,25);
  BG_PALETTE[CERA2] = RGB15(29,27,25);
  BG_PALETTE[UNID] = RGB15(31,0,31);
  BG_PALETTE[UNIDT] = RGB15(31,0,0);
  BG_PALETTE[OIL] = RGB15(16,8,8);
  BG_PALETTE[SWATER] = RGB15(8,16,31);
  BG_PALETTE[SALT] = RGB15(31,31,31);
  BG_PALETTE[SNOW] = RGB15(25,25,25);
  BG_PALETTE[STEAM] = RGB15(17,17,17);
  BG_PALETTE[CONDEN] = RGB15(20,20,20);

  // --**ooOO- Sub BG -OOoo**--

  videoSetModeSub(MODE_0_2D | DISPLAY_BG0_ACTIVE |
                  DISPLAY_SPR_ACTIVE | DISPLAY_SPR_1D);
  vramSetBankC(VRAM_C_SUB_BG);
  vramSetBankD(VRAM_D_SUB_SPRITE);

  SUB_BG0_CR = BG_TILE_BASE(0) | BG_MAP_BASE(31) | BG_32x32 | BG_256_COLOR;
  //memset32(BG_MAP_RAM_SUB(31), 0, 32*32/4);

  // load brush tiles
  //u32 len = 0;
  //u16* data = (u16*) gbfs_get_obj(gbfs_file, "brushes.bin", &len);
  memcpy32(BG_GFX_SUB, brushes_bin, brushes_bin_size>>2);
  //data = (u16*) gbfs_get_obj(gbfs_file, "brushes.pal.bin", &len);
  memcpy16(BG_PALETTE_SUB, brushes_pal_bin, brushes_pal_bin_size>>1);

  // load selector sprite
  //data = (u16*) gbfs_get_obj(gbfs_file, "selector.bin", &len);
  memcpy32(SPRITE_GFX_SUB, selector_bin, selector_bin_size>>2);
  //data = (u16*) gbfs_get_obj(gbfs_file, "selector.pal.bin", &len);
  memcpy16(SPRITE_PALETTE_SUB, selector_pal_bin, selector_pal_bin_size>>1);

  SpriteRotation *selectorrot = &oam_back_rot[0];
  initOAM(selectorrot);
  SpriteEntry *selector = &oam_back[0];
  selector[0].attribute[0] = ATTR0_ROTSCALE_DOUBLE | ATTR0_COLOR_256;
  selector[0].attribute[1] = ATTR1_ROTDATA(0) | ATTR1_SIZE_16;
  selector[0].attribute[2] = 0 | ATTR2_PRIORITY(0);
  moveSprite(&selector[0], 24, 112);
  selector[1].attribute[0] = ATTR0_COLOR_256;
  selector[1].attribute[1] = ATTR1_SIZE_8;
  selector[1].attribute[2] = 8;
  moveSprite(&selector[1], 256-24-10, 191-4-16*4);

  u16 selectorangle = 0;
  initOAM((SpriteRotation*)OAM_SUB);

  // lay out them brush tiles
  //data = (u16*) gbfs_get_obj(gbfs_file, "map.bin", &len);
  memcpy16((u16*)BG_MAP_RAM_SUB(31), map_bin, map_bin_size>>1);

  /*************
   * Main Loop *
   *************/

  u8 selected = 1;
  u8 brushes[] = {NOTHING, WALL,  SAND, SNOW,
                  WATER,   PLANT, SALT, SPOUT,
                  OIL,     FIRE,  CERA, UNID};
  u8 thickness = 0;
  u32 thicknesses[] = {2,4,6,8};

  u32 framecounter = 0;
  int touched_last = 0;
  s16 lastx=0,lasty=0;
  swiWaitForVBlank(); // wait for things to settle down (hopefully)
  init_genrand(IPC->rtc_seconds + IPC->rtc_minutes * 64 + IPC->rtc_hours * 4096);
	while(1) {
    swiWaitForVBlank();
    { // zot the sprite backbuffer to OAM
      int i;
      for (i = 0; i < 4; i++)
        ((SpriteRotation*)OAM_SUB)[i] = oam_back_rot[i];
    }

		touch=touchReadXY();
    scanKeys();
    u32 held = keysHeld(),
        pressed = keysDown();

    if (touched_last) {
      bresenThick(buf, lastx, lasty, touch.px, touch.py,
          brushes[selected], thicknesses[thickness] << 16);
      bresenCircle(buf, lastx, lasty,
          thicknesses[thickness] >> 1, brushes[selected]);
      bresenCircle(buf, touch.px, touch.py,
          thicknesses[thickness] >> 1, brushes[selected]);
    }

    // ERASER WALL SAND
    // WATER  TREE SALT
    // OIL    FIRE WAX  ???

    if (pressed & KEY_LEFT) {
      if (selected % 4 != 0) {
        selected -= 1;
        moveSprite(&selector[0], (selected % 4)*24, 112+(selected/4)*24);
      }
    }

    if (pressed & KEY_RIGHT) {
      if (selected % 4 != 3) {
        selected += 1;
        moveSprite(&selector[0], (selected % 4)*24, 112+(selected/4)*24);
      }
    }

    if (pressed & KEY_UP) {
      if (selected > 3) {
        selected -= 4;
        moveSprite(&selector[0], (selected % 4)*24, 112+(selected/4)*24);
      }
    }

    if (pressed & KEY_DOWN) {
      if (selected < 8) {
        selected += 4;
        moveSprite(&selector[0], (selected % 4)*24, 112+(selected/4)*24);
      }
    }

    if (pressed & KEY_X) {
      if (thickness > 0) {
        thickness--;
        moveSprite(&selector[1], 256-24-10, 191-4-16*(4-thickness));
      }
    }

    if (pressed & KEY_Y) {
      if (thickness < 3) {
        thickness++;
        moveSprite(&selector[1], 256-24-10, 191-4-16*(4-thickness));
      }
    }

    if (held & KEY_TOUCH) {
      if (touch.px != 0 && touch.py != 0) { 
        touched_last = 1;
        lastx = touch.px;
        lasty = touch.py;
      }
    } else {
      touched_last = 0;
    }

    spawn(buf);
    calculate(buf);
    memcpy32(BG_GFX, buf, (256*192)>>2);

    selectorangle += 10;
    selectorangle &= 0x1ff;
    rotSprite(selectorrot, selectorangle);

    // rotate the palette of the thickness selector arrow thingy
    SPRITE_PALETTE_SUB[1] = arrow_pal_rot[framecounter];
    SPRITE_PALETTE_SUB[2] = arrow_pal_rot[framecounter+1];
    SPRITE_PALETTE_SUB[3] = arrow_pal_rot[framecounter+2];
    framecounter += 1;
    if (framecounter >= 7) framecounter = 0;
	}

	return 0;
}
