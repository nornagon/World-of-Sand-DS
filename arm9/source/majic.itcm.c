#include "majic.h"

//#define CHANCE(n) (genrand_int8() < ((u32)((n)*0xff)))
#define CHANCE(n) (genrand_int32() < ((u32)((n)*0xffffffff)))

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


u32 ITCM_CODE majic(u8* buf, u32 x, u32 y) {
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
      } else if (CHANCE(0.25)) {
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
        // better sinking
        if (CHANCE(0.5)) {
          if (bot[0] == WATER) { bot[0] = SWATER; mid[1] = WATER; break; }
          if (bot[2] == WATER) { bot[2] = SWATER; mid[1] = WATER; break; }
          if (mid[2] == WATER) { mid[2] = SWATER; mid[1] = WATER; break; }
          if (mid[0] == WATER) { mid[0] = SWATER; mid[1] = WATER; break; }
        } else {
          if (bot[2] == WATER) { bot[2] = SWATER; mid[1] = WATER; break; }
          if (bot[0] == WATER) { bot[0] = SWATER; mid[1] = WATER; break; }
          if (mid[0] == WATER) { mid[0] = SWATER; mid[1] = WATER; break; }
          if (mid[2] == WATER) { mid[2] = SWATER; mid[1] = WATER; break; }
        }
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
      /*if (CHANCE(0.1) && top[1] == NOTHING) {
        top[1] = SNOW;
        mid[1] = NOTHING; break;
      }*/
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
    default:
      return 0;
  }
  return 1;
}