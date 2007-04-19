/*------------------------------------------------------------------------------
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


------------------------------------------------------------------------------*/
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
#include "font_bin.h"

#include "majic.h"
#include "comms.h"

#define REPEAT_U8_TO_U32(n) ((n) | ((n)<<8) | ((n)<<16) | ((n)<<24))

volatile bool have_data_from_arm7 = true;
volatile u32 data_from_arm7_len = 0;

void arm7_data_handler() {
  u32 msg = REG_IPC_FIFO_RX;
  if ((msg & ~0xffff) == COMMS_DATA_AVAILABLE) {
    u32 len = msg & 0xffff;
    WRAM_CR = 0; // 32K to ARM9
    have_data_from_arm7 = true;
    data_from_arm7_len = len;
  }
}

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
  // magic constants signify x-value of centre of falls
  addsome(SAND, buf, 51);
  addsome(WATER, buf, 102);
  addsome(SALT, buf, 153);
  addsome(OIL, buf, 204);
}

u32 calculate(u8* buf, u8* inbuf) {
  static int counter = 0, arm7lines = 0;
  u32 x,y;
  u32 particount = 0;
  genrand_regen();

  // 0. read data from arm7 (frame n)
  // 1. run stitching line (frame n+1)
  // 2. apply input to lower portion (frame n+1)
  // 3. send lower portion to arm7 (frame n+1)
  // 4. calculate continuing from previous arm7 data (frame n)
  // 5. render (frame n)
  // 6. apply input to top portion (frame n+1)
  // 7. n++; goto 1

  // read data from arm7, we'll continue from it
  while (!have_data_from_arm7);
  memcpy(buf+130*256, WRAM, (191-130)*256);
  particount += data_from_arm7_len;
  //iprintf("\x1b[0;0H%04x", data_from_arm7_len);
  have_data_from_arm7 = false;

  // run a stitching line
  if (counter) { // left to right
    for (x = 1; x < 255; x++)
      majic(buf, x, 130);
    for (x = 1; x < 255; x++)
      majic(buf, x, 129);
  } else {
    for (x = 255; x > 1; x--)
      majic(buf, x, 130);
    for (x = 255; x > 1; x--)
      majic(buf, x, 129);
  }

  // apply input to arm7 part of buffer (input is for frame n+1)
  for (y = 130; y < 191; y++)
    for (x = 1; x < 255; x++)
      if (inbuf[y*256+x] != NUM_MATERIALS)
        buf[y*256+x] = inbuf[y*256+x];
  memset32(inbuf+130*256, REPEAT_U8_TO_U32(NUM_MATERIALS), ((191-130)*256)>>2);

  // copy data back to arm7.
  put_data(buf+130*256, (192-130)*256);

  // continue from where the arm7 left off
  if (counter)
    for (y = 128; y > 0; y--) // ^
      for (x = 1; x < 255; x++) // ->
        particount += majic(buf,x,y);
  else
    for (y = 128; y > 0; y--) // ^
      for (x = 255; x > 0; x--) // <-
        particount += majic(buf,x,y);
  // next time calculate() is called, the arm7 should have more data for us

  // apply input to arm9 part of the data (for consideration in frame n+1)
  for (y = 1; y < 130; y++)
    for (x = 1; x < 255; x++)
      if (inbuf[y*256+x] != NUM_MATERIALS)
        buf[y*256+x] = inbuf[y*256+x];
  memset32(inbuf, REPEAT_U8_TO_U32(NUM_MATERIALS), (130*256)>>2);

  // drop a black rectangle over it all
  memset32(buf, NOTHING, 256>>2);
  memset32(buf+191*256, NOTHING, 256>>2);
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

//------------------------------------------------------------------------------
int main(void) {
//------------------------------------------------------------------------------
	touchPosition touch;
  u8 *buf = malloc(256*192), *inputbuf = malloc(256*192);
  memset32(buf, REPEAT_U8_TO_U32(NOTHING), (256*192)>>2);
  memset32(inputbuf, REPEAT_U8_TO_U32(NUM_MATERIALS), (256*192)>>2);
  WRAM_CR = 0; // 32K to ARM9
  memset32(WRAM, REPEAT_U8_TO_U32(NOTHING), (32*1024)>>2);

  /********
   * IRQs *
   ********/

  irqInit();
  irqSet(IRQ_VBLANK, vblank_counter);
  REG_IPC_FIFO_CR = IPC_FIFO_ENABLE | IPC_FIFO_SEND_CLEAR;
  irqSet(IRQ_FIFO_NOT_EMPTY, arm7_data_handler);
  irqEnable(IRQ_VBLANK | IRQ_FIFO_NOT_EMPTY);
  REG_IPC_FIFO_CR = IPC_FIFO_ENABLE | IPC_FIFO_RECV_IRQ;

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
  BG_PALETTE[STEAM] = RGB15(17,17,20);
  BG_PALETTE[CONDEN] = RGB15(20,20,23);
  BG_PALETTE[ACID] = RGB15(12,31,19);
  BG_PALETTE[LIQFIRE] = RGB15(31,27,14);
  BG_PALETTE[TORCH] = RGB15(31,27,0);
  BG_PALETTE[CONCRETE] = RGB15(18,18,18);
  BG_PALETTE[ANTIMATTER] = RGB15(1,4,1);
  BG_PALETTE[ANTIMATTER2] = RGB15(31,9,9);
  BG_PALETTE[MUD] = RGB15(22,7,0);

  // --**ooOO- Sub BG -OOoo**--

  videoSetModeSub(MODE_0_2D | DISPLAY_BG0_ACTIVE | DISPLAY_BG1_ACTIVE |
                  DISPLAY_SPR_ACTIVE | DISPLAY_SPR_1D);
  vramSetBankC(VRAM_C_SUB_BG);
  vramSetBankD(VRAM_D_SUB_SPRITE);

  SUB_BG0_CR = BG_TILE_BASE(0) | BG_MAP_BASE(31) | BG_32x32 | BG_256_COLOR |
    BG_PRIORITY(2);
  //memset32(BG_MAP_RAM_SUB(31), 0, 32*32/4);

  // load brush tiles
  //
  // v-- gbfs code
  //u32 len = 0;
  //u16* data = (u16*) gbfs_get_obj(gbfs_file, "brushes.bin", &len);
  //data = (u16*) gbfs_get_obj(gbfs_file, "brushes.pal.bin", &len);
  //
  // v-- not gbfs code
  memcpy32(BG_GFX_SUB, brushes_bin, brushes_bin_size>>2);
  memcpy16(BG_PALETTE_SUB, brushes_pal_bin, brushes_pal_bin_size>>1);

  // --**ooOO- Selectors -OOoo**--

  // load selector sprite
  //data = (u16*) gbfs_get_obj(gbfs_file, "selector.bin", &len);
  //data = (u16*) gbfs_get_obj(gbfs_file, "selector.pal.bin", &len);
  memcpy32(SPRITE_GFX_SUB, selector_bin, selector_bin_size>>2);
  memcpy16(SPRITE_PALETTE_SUB, selector_pal_bin, selector_pal_bin_size>>1);

  SpriteRotation *selectorrot = &oam_back_rot[0];
  initOAM(selectorrot);
  SpriteEntry *selector = &oam_back[0];
  selector[0].attribute[0] = ATTR0_ROTSCALE_DOUBLE | ATTR0_COLOR_256;
  selector[0].attribute[1] = ATTR1_ROTDATA(0) | ATTR1_SIZE_16;
  selector[0].attribute[2] = 0 | ATTR2_PRIORITY(0);
  moveSprite(&selector[0], 24, 256-21*8);
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

  SUB_BG1_CR = BG_TILE_BASE(1) | BG_MAP_BASE(29) | BG_PRIORITY(0) | BG_256_COLOR;
  BG_PALETTE_SUB[255] = RGB15(31,31,31);
  BG_PALETTE_SUB[254] = RGB15(31,27,0);

  consoleInit((u16*)font_bin, (u16*)CHAR_BASE_BLOCK_SUB(1), 102, 32, (u16*)SCREEN_BASE_BLOCK_SUB(29), CONSOLE_USE_COLOR255, 8);

  memcpy16((u16*)CHAR_BASE_BLOCK_SUB(1), font_bin, font_bin_size>>1);

  /********
   * Help *
   ********/

  iprintf("\x1b[3;1HUse the arrow keys to select");
  iprintf("\x1b[4;1Hmaterial. Use the \x80 and \x81 keys");
  iprintf("\x1b[5;1Hto change brush thickness.");
  iprintf("\x1b[6;1HHold \x82 and \x83\x84\x85 on the sources");
  iprintf("\x1b[7;1Hto toggle them.");

  /*************
   * Main Loop *
   *************/

  u8 selected = 1;
  u8 brushes[] = {NOTHING, WALL,     SAND, SNOW,
                  WATER,   PLANT,    SALT, SPOUT,
                  OIL,     FIRE,     CERA, UNID,
                  LIQFIRE, CONCRETE, ACID, ANTIMATTER,};
  u8 thickness = 0;
  u32 thicknesses[] = {2,4,6,8};

  int sand_on = 1,
      water_on = 1,
      salt_on = 1,
      oil_on = 1;

  u32 framecounter = 0;
  u32 particount = 0, fps = 0;
  int touched_last = 0;
  s16 lastx=0,lasty=0;
  swiWaitForVBlank(); // wait for things to settle down (hopefully)
  init_genrand(*((u32*)IPC->curtime)); // XXX: danger will robinson; IPC is to be removed
	while (1) {
    if (particount < 700)
      swiWaitForVBlank();
    { // zot the sprite backbuffer to OAM
      // FIXME: um, why isn't this a loop? ISTR it not working that way for some weeeeird reason
      ((SpriteRotation*)OAM_SUB)[0] = oam_back_rot[0];
      ((SpriteRotation*)OAM_SUB)[1] = oam_back_rot[1];
      //((SpriteRotation*)OAM_SUB)[2] = oam_back_rot[2];
      //((SpriteRotation*)OAM_SUB)[3] = oam_back_rot[3];
    }

		touch = touchReadXY();
    scanKeys();
    u32 held = keysHeld(),
        pressed = keysDown();

    if (touched_last) {
      bresenThick(inputbuf, lastx, lasty, touch.px, touch.py,
          brushes[selected], thicknesses[thickness] << 16);
      bresenCircle(inputbuf, lastx, lasty,
          thicknesses[thickness] >> 1, brushes[selected]);
      bresenCircle(inputbuf, touch.px, touch.py,
          thicknesses[thickness] >> 1, brushes[selected]);
    }

    if (pressed & KEY_LEFT) {
      if (selected % 4 != 0) {
        selected -= 1;
        moveSprite(&selector[0], (selected % 4)*24, (256-21*8)+(selected/4)*24);
      }
    }

    if (pressed & KEY_RIGHT) {
      if (selected % 4 != 3) {
        selected += 1;
        moveSprite(&selector[0], (selected % 4)*24, (256-21*8)+(selected/4)*24);
      }
    }

    if (pressed & KEY_UP) {
      if (selected > 3) {
        selected -= 4;
        moveSprite(&selector[0], (selected % 4)*24, (256-21*8)+(selected/4)*24);
      }
    }

    if (pressed & KEY_DOWN) {
      if (selected < sizeof(brushes)/sizeof(u8)-4) {
        selected += 4;
        moveSprite(&selector[0], (selected % 4)*24, (256-21*8)+(selected/4)*24);
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

    if (held & KEY_L) {
      if ((pressed & KEY_TOUCH) && (touch.py <= 7)) {
        if (touch.px >= 45 && touch.px <= 57)
          sand_on = !sand_on;
        else if (touch.px >= 96 && touch.px <= 108)
          water_on = !water_on;
        else if (touch.px >= 147 && touch.px <= 159)
          salt_on = !salt_on;
        else if (touch.px >= 198 && touch.px <= 210)
          oil_on = !oil_on;
      }
    } else if (held & KEY_TOUCH) {
      if (touch.px != 0 && touch.py != 0) { 
        {
          touched_last = 1;
          lastx = touch.px;
          lasty = touch.py;
        }
      }
    }
    if (!(held & KEY_TOUCH)) {
      touched_last = 0;
    }

    if (sand_on) addsome(SAND, buf, 51);
    if (water_on) addsome(WATER, buf, 102);
    if (salt_on) addsome(SALT, buf, 153);
    if (oil_on) addsome(OIL, buf, 204);

    particount = calculate(buf, inputbuf);
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
      iprintf("\x1b[0;0HParticles: %5d", particount);
      vblnks = frames = 0;
    }
	}

	return 0;
}
