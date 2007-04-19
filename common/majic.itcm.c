#include "majic.h"

// this is a sneaky way of letting us use real probability values in our algos
// (0 <= x <= 1) and having gcc convert it to integer arith at compile time.
#define CHANCE(n) (genrand_int32() < ((u32)((n)*0xffffffff)))

// these LUTs are faster than doing if (a || b || c || d || e...)

// steam rises through it, ash falls through it, etc.
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
  true, // ACID
  true, // LIQFIRE
  false, // TORCH
  true, // CONCRETE
  false, // ANTIMATTER
  false, // ANTIMATTER2
  false, // MUD
};

// steam condenses on the bottom of it
bool SOLID[NUM_MATERIALS] = {
  false, // NOTHING
  false, // SAND
  false, // WATER
  false, // FIRE
  true, // WALL
  true, // PLANT
  false, // SMOKE
  false, // ASH
  true, // SPOUT
  true, // CERA
  false, // CERA2
  false, // UNID
  false, // UNIDT
  false, // OIL
  false, // SWATER
  false, // SALT
  false, // SNOW
  false, // STEAM
  false, // CONDEN
  false, // ACID
  false, // LIQFIRE
  true, // TORCH
  false, // CONCRETE
  false, // ANTIMATTER
  false, // ANTIMATTER2
  false, // MUD
};

// acid burns through it
bool ACIDBURNT[NUM_MATERIALS] = {
  false, // NOTHING
  true, // SAND
  false, // WATER
  false, // FIRE
  true, // WALL
  true, // PLANT
  false, // SMOKE
  true, // ASH
  true, // SPOUT
  false, // CERA
  false, // CERA2
  false, // UNID
  false, // UNIDT
  false, // OIL
  true, // SWATER
  false, // SALT
  true, // SNOW
  false, // STEAM
  false, // CONDEN
  false, // ACID
  false, // LIQFIRE
  true, // TORCH
  true, // CONCRETE
  false, // ANTIMATTER
  false, // ANTIMATTER2
  true, // MUD
};

// standard gravity, put here in a macro to save space
// NOTE that you ABSOLUTELY MUST put braces around this. If you do not, the
// gods will kill a kitten (and your code will break).
#define FALL(mat) \
  if (bot[1] == NOTHING) { mid[1] = NOTHING; bot[1] = mat; break; } \
  if (bot[0] == NOTHING) { mid[1] = NOTHING; bot[0] = mat; break; } \
  if (bot[2] == NOTHING) { mid[1] = NOTHING; bot[2] = mat; break; } \
  if (mid[2] == NOTHING) { mid[1] = NOTHING; mid[2] = mat; break; } \
  if (mid[0] == NOTHING) { mid[1] = NOTHING; mid[0] = mat; break; }
// XXX: We can't use the do { } while (0) trick here because the break needs to break the switch.

u32 ITCM_CODE majic(u8* buf, u32 x, u32 y) {
  // the top and bottom rows of three pixels directly above and below (x,y)
  // are loaded for easy access.
  u8* top = buf+(x-1)+(y-1)*256,
    * mid = buf+(x-1)+(y)*256,
    * bot = buf+(x-1)+(y+1)*256;
  // ttt
  // mxm
  // bbb
  // px = mid[1]
  switch (mid[1]) {
    case SAND:
      if (CHANCE(0.95)) { FALL(SAND); }
      if (CHANCE(0.25)) {
        // sink below water
        if (bot[1] == WATER) { bot[1] = SAND; mid[1] = WATER; break; }
        // don't favour one side over the other
        if (CHANCE(0.5)) {
          if (bot[0] == WATER) { bot[0] = SAND; mid[1] = WATER; break; }
          if (bot[2] == WATER) { bot[2] = SAND; mid[1] = WATER; break; }
        } else {
          if (bot[2] == WATER) { bot[2] = SAND; mid[1] = WATER; break; }
          if (bot[0] == WATER) { bot[0] = SAND; mid[1] = WATER; break; }
        }
      }
      break;

    case WATER:
      if (CHANCE(0.95)) { FALL(WATER); }
      break;

    case SWATER:
      if (CHANCE(0.95)) { FALL(SWATER); }
      if (bot[1] == WATER && CHANCE(0.5)) {
        // sink below water
        bot[1] = SWATER;
        mid[1] = WATER;
        break;
      }
      if (CHANCE(0.3)) {
        // better sinking
        if (CHANCE(0.5)) { // direction-unweighted
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
      { mid[1] = NOTHING; break; }
      if (CHANCE(0.9)) {
        // water + fire = steam
        if (top[1] == WATER) { top[1] = STEAM; mid[1] = NOTHING; break; }
        if (mid[2] == WATER) { mid[2] = STEAM; mid[1] = NOTHING; break; }
        if (mid[0] == WATER) { mid[0] = STEAM; mid[1] = NOTHING; break; }
        if (bot[1] == WATER) { bot[1] = STEAM; mid[1] = NOTHING; break; }
        // salt water + fire = steam + salt
        if (top[1] == SWATER) { top[1] = STEAM; mid[1] = SALT; break; }
        if (mid[2] == SWATER) { mid[2] = STEAM; mid[1] = SALT; break; }
        if (mid[0] == SWATER) { mid[0] = STEAM; mid[1] = SALT; break; }
        if (bot[1] == SWATER) { bot[1] = STEAM; mid[1] = SALT; break; }
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
      if (CHANCE(0.6)) break;
      if (CHANCE(0.6)) {
        if (CHANCE(0.5)) { // direction-neutral; float up
          if (top[0] == NOTHING) { top[0] = SMOKE; mid[1] = NOTHING; break; }
          if (top[2] == NOTHING) { top[2] = SMOKE; mid[1] = NOTHING; break; }
        } else {
          if (top[2] == NOTHING) { top[2] = SMOKE; mid[1] = NOTHING; break; }
          if (top[0] == NOTHING) { top[0] = SMOKE; mid[1] = NOTHING; break; }
        }
      } else
        if (top[1] == NOTHING) { top[1] = SMOKE; mid[1] = NOTHING; break; }

      if (CHANCE(0.01)) { mid[1] = ASH; break; } // stop floating up
      break;

    case ASH:
      if (CHANCE(0.2)) break;

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
      }

      // ash + water = mud
      if (bot[1] == WATER) { bot[1] = MUD; mid[1] = NOTHING; break; }
      if (bot[0] == WATER) { bot[0] = MUD; mid[1] = NOTHING; break; }
      if (bot[2] == WATER) { bot[2] = MUD; mid[1] = NOTHING; break; }
      if (mid[0] == WATER) { mid[0] = MUD; mid[1] = NOTHING; break; }
      if (mid[2] == WATER) { mid[2] = MUD; mid[1] = NOTHING; break; }
      if (top[1] == WATER) { top[1] = MUD; mid[1] = NOTHING; break; }

      // ash falls through liquids
      if (bot[1] == NOTHING || LIQUID[bot[1]])
        { mid[1] = bot[1]; bot[1] = ASH; break; }
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
      if (CHANCE(0.95)) { FALL(OIL); }
      if (CHANCE(0.95)) {
        // rise above water
        if (top[1] == WATER) { top[1] = OIL; mid[1] = WATER; break; }
        if (top[0] == WATER) { top[0] = OIL; mid[1] = WATER; break; } // TODO: direction-neutrality
        if (top[2] == WATER) { top[2] = OIL; mid[1] = WATER; break; }
      }
      if (CHANCE(0.3)) {
        if (mid[2] == WATER) { mid[2] = OIL; mid[1] = WATER; break; }
        if (mid[0] == WATER) { mid[0] = OIL; mid[1] = WATER; break; }
      }
      break;

    case SALT:
      if (CHANCE(0.95)) { FALL(SALT); }
      if (CHANCE(0.9)) { // salt water creation
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
        mid[1] = WATER; break; // melty
      }
      if (CHANCE(0.9)) {
        // floaty
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
        // rise up
        if (top[1] == NOTHING || LIQUID[top[1]]) {
          mid[1] = top[1]; top[1] = STEAM; break; }
        if (top[0] == NOTHING || LIQUID[top[0]]) {
          mid[1] = top[0]; top[0] = STEAM; break; }
        if (top[2] == NOTHING || LIQUID[top[2]]) {   // ivy :)
          mid[1] = top[2]; top[2] = STEAM; break; }
        if (mid[0] == NOTHING) { mid[0] = STEAM; mid[1] = NOTHING; break; }
        if (mid[2] == NOTHING) { mid[2] = STEAM; mid[1] = NOTHING; break; }

        // ash turns steam into water... TODO: mud?
        if (top[1] == ASH) { mid[1] = WATER; break; }
        if (mid[0] == ASH) { mid[1] = WATER; break; }
        if (mid[2] == ASH) { mid[1] = WATER; break; }
        if (bot[1] == ASH) { mid[1] = WATER; break; }
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

    case ACID:
      if (CHANCE(0.95)) { FALL(ACID); }
      if (bot[1] == LIQFIRE) { mid[1] = NOTHING; bot[1] = ANTIMATTER; break; }
      if (mid[0] == LIQFIRE) { mid[1] = NOTHING; mid[0] = ANTIMATTER; break; }
      if (mid[2] == LIQFIRE) { mid[1] = NOTHING; mid[2] = ANTIMATTER; break; }
      if (top[1] == LIQFIRE) { mid[1] = NOTHING; top[1] = ANTIMATTER; break; }
      if (CHANCE(0.5)) {
        register u32 dir = CHANCE(0.5) ? 0 : 2;
        register u32 otherdir = dir ? 0 : 2;
        if (ACIDBURNT[bot[1]]) {
          mid[1] = NOTHING; bot[1] = NOTHING; break; }
        if (ACIDBURNT[bot[dir]]) {
          mid[1] = NOTHING; bot[dir] = NOTHING; break; }
        if (ACIDBURNT[bot[otherdir]]) {
          mid[1] = NOTHING; bot[otherdir] = NOTHING; break; }
        if (ACIDBURNT[mid[dir]]) {
          mid[1] = NOTHING; mid[otherdir] = NOTHING; break; }
        if (ACIDBURNT[mid[otherdir]]) {
          mid[1] = NOTHING; mid[otherdir] = NOTHING; break; }
        if (CHANCE(0.1)) {
          if (ACIDBURNT[top[1]]) {
            top[1] = NOTHING; mid[1] = NOTHING; break; }
          if (ACIDBURNT[top[dir]]) {
            top[dir] = NOTHING; mid[1] = NOTHING; break; }
          if (ACIDBURNT[top[otherdir]]) {
            top[otherdir] = NOTHING; mid[1] = NOTHING; break; }
        }
      }
      break;

    case LIQFIRE:
      if (CHANCE(0.95)) { FALL(LIQFIRE); }
      if (CHANCE(0.7) && top[1] == NOTHING) { top[1] = FIRE; break; }
      if (CHANCE(0.001)) { mid[1] = NOTHING; break; } // fizzle out
      if (CHANCE(0.7)) {
        if (bot[1] == CERA) { mid[1] = NOTHING; bot[1] = TORCH; break; }
        if (mid[0] == CERA) { mid[1] = NOTHING; mid[0] = TORCH; break; }
        if (mid[2] == CERA) { mid[1] = NOTHING; mid[2] = TORCH; break; }
        if (top[1] == CERA) { mid[1] = NOTHING; top[1] = TORCH; break; }
      }
      break;
    case TORCH:
      if (CHANCE(0.75)) {
        if (top[1] == NOTHING) { top[1] = FIRE; break; }
        if (mid[0] == NOTHING) { mid[0] = FIRE; break; }
        if (mid[2] == NOTHING) { mid[2] = FIRE; break; }
      }
      if (top[1] == SAND) { mid[1] = NOTHING; break; }
      if (mid[0] == SAND) { mid[1] = NOTHING; break; }
      if (mid[2] == SAND) { mid[1] = NOTHING; break; }
      if (bot[1] == SAND) { mid[1] = NOTHING; break; }
      break;

    case ANTIMATTER: {
      u32 dir = CHANCE(0.5) ? 0 : 2;
      u32 otherdir = dir ? 0 : 2;
      if (CHANCE(0.95)) { FALL(ANTIMATTER); }
      if ((bot[1] != NOTHING && bot[1] != ANTIMATTER && bot[1] != FIRE && bot[1] != ANTIMATTER2)
          || (mid[dir] != NOTHING && mid[dir] != ANTIMATTER && mid[dir] != FIRE && mid[dir] != ANTIMATTER2)
          || (mid[otherdir] != NOTHING && mid[otherdir] != ANTIMATTER && mid[otherdir] != FIRE && mid[otherdir] != ANTIMATTER2)
          || (top[1] != NOTHING && top[1] != ANTIMATTER && top[1] != FIRE && top[1] != ANTIMATTER2)) {
        bot[1] = mid[0] = mid[1] = mid[2] = top[1] = ANTIMATTER2;
      }
    } break;
    case ANTIMATTER2:
      if (CHANCE(0.5)) { mid[1] = FIRE; break; }
      if (CHANCE(0.3)) { top[1] = mid[0] = mid[2] = bot[1] = ANTIMATTER2; break; }
      break;

    case CONCRETE:
      if (CHANCE(0.01) && (SOLID[bot[1]] || SOLID[mid[0]] || SOLID[mid[2]] || SOLID[top[1]])) {
        mid[1] = WALL; break; }
      if (CHANCE(0.95)) { FALL(CONCRETE); }
      break;

    case MUD:
      if (CHANCE(0.95)) { FALL(MUD); }
      if (CHANCE(0.25)) {
        // sink below water
        if (bot[1] == WATER) { bot[1] = MUD; mid[1] = WATER; break; }
        if (CHANCE(0.5)) {
          if (bot[0] == WATER) { bot[0] = MUD; mid[1] = WATER; break; }
          if (bot[2] == WATER) { bot[2] = MUD; mid[1] = WATER; break; }
        } else {
          if (bot[2] == WATER) { bot[2] = MUD; mid[1] = WATER; break; }
          if (bot[0] == WATER) { bot[0] = MUD; mid[1] = WATER; break; }
        }
      }
      break;

    default: // NOTHING, WALL; return 0 for means of particount
      return 0;
  }
  return 1;
}
