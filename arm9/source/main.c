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

#include "majic.h"


#define CHANCE(n) (genrand_int32() < ((u32)((n)*0xffffffff)))

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

u32 calculate(u8* buf) {
  static int counter = 0;
  u32 x,y;
  u32 particount = 0;
  genrand_regen();

  if (counter)
    for (y = 191; y > 0; y--) // ^
      for (x = 1; x < 255; x++) // ->
        particount += majic(buf,x,y);
  else
    for (y = 191; y > 0; y--) // ^
      for (x = 255; x > 0; x--) // <-
        particount += majic(buf,x,y);
  // drop a black rectangle over it all
  memset32(buf, NOTHING, 64);
  memset32(buf+192*256, NOTHING, 64);
  for (y = 1; y < 192; y++)
    buf[y*256] = buf[y*256+255] = NOTHING;
  
  counter = !counter; // go the other way next time
  return particount;
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

u32 vblnks = 0, frames = 0;
void vblank_counter() {
  vblnks += 1;
}

//---------------------------------------------------------------------------------
int main(void) {
//---------------------------------------------------------------------------------
	touchPosition touch;
  u8 *buf = malloc(256*192);
  memset(buf, 0, 256*192);

  irqInit();
  irqEnable(IRQ_VBLANK);
  irqSet(IRQ_VBLANK, vblank_counter);

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
  BG_PALETTE[SNOW] = RGB15(28,28,28);
  BG_PALETTE[STEAM] = RGB15(17,17,17);
  BG_PALETTE[CONDEN] = RGB15(20,20,20);

  // --**ooOO- Sub BG -OOoo**--

  videoSetModeSub(MODE_0_2D | DISPLAY_BG0_ACTIVE | DISPLAY_BG1_ACTIVE |
                  DISPLAY_SPR_ACTIVE | DISPLAY_SPR_1D);
  vramSetBankC(VRAM_C_SUB_BG);
  vramSetBankD(VRAM_D_SUB_SPRITE);

  SUB_BG0_CR = BG_TILE_BASE(0) | BG_MAP_BASE(31) | BG_32x32 | BG_256_COLOR |
    BG_PRIORITY(2);
  //memset32(BG_MAP_RAM_SUB(31), 0, 32*32/4);

  // load brush tiles
  //u32 len = 0;
  //u16* data = (u16*) gbfs_get_obj(gbfs_file, "brushes.bin", &len);
  memcpy32(BG_GFX_SUB, brushes_bin, brushes_bin_size>>2);
  //data = (u16*) gbfs_get_obj(gbfs_file, "brushes.pal.bin", &len);
  memcpy16(BG_PALETTE_SUB, brushes_pal_bin, brushes_pal_bin_size>>1);

  // --**ooOO- Selectors -OOoo**--

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

  // --**ooOO- Font -OOoo**--

  SUB_BG1_CR = BG_TILE_BASE(1) | BG_MAP_BASE(29) | BG_PRIORITY(0) | BG_16_COLOR;
  BG_PALETTE_SUB[255] = RGB15(31,31,31);

  consoleInitDefault((u16*)SCREEN_BASE_BLOCK_SUB(29), (u16*)CHAR_BASE_BLOCK_SUB(1), 16);

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
  u32 particount = 0, fps = 0;
  int touched_last = 0;
  s16 lastx=0,lasty=0;
  swiWaitForVBlank(); // wait for things to settle down (hopefully)
  init_genrand(IPC->rtc_seconds + IPC->rtc_minutes * 64 + IPC->rtc_hours * 4096);
	while (1) {
    if (particount < 700)
      swiWaitForVBlank();
    { // zot the sprite backbuffer to OAM
      ((SpriteRotation*)OAM_SUB)[0] = oam_back_rot[0];
      ((SpriteRotation*)OAM_SUB)[1] = oam_back_rot[1];
      //((SpriteRotation*)OAM_SUB)[2] = oam_back_rot[2];
      //((SpriteRotation*)OAM_SUB)[3] = oam_back_rot[3];
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
    particount = calculate(buf);
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

    frames += 1;
    if (vblnks >= 60) {
      iprintf("\x1b[0;26H%2dfps", fps = (frames * 64 - frames * 4) / vblnks);
      iprintf("\x1b[1;20H%10d", particount);
      vblnks = frames = 0;
    }
	}

	return 0;
}
