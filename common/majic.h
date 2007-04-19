#ifndef MAJIC_H
#define MAJIC_H

#include <nds.h>
#include "mt19937ar.h" // mersenne twister

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
  ACID,
  LIQFIRE,
  TORCH,
  CONCRETE,
  ANTIMATTER,
  ANTIMATTER2,
  MUD,
  NUM_MATERIALS
} MATERIAL;


//#ifdef ARM7
//#define ITCM_CODE
//#endif

u32 ITCM_CODE majic(u8* buf, u32 x, u32 y);

#endif // MAJIC_H
