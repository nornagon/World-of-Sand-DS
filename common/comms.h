#ifndef COMMS_H
#define COMMS_H

#include <nds.h>
#include <string.h>
#include <stdio.h>

#define WRAM ((vuint8*)0x037f8000)
#define COMMS_DATA_AVAILABLE 0xcdcd0000

#ifdef ARM9
void put_data(u8* data, u32 len) {
  WRAM_CR = 0; // 32K to ARM9
  if (len > 32*1024) len = 32*1024;
  memcpy((void*)WRAM, data, len);
  WRAM_CR = 3; // 32K to ARM7
  REG_IPC_FIFO_TX = COMMS_DATA_AVAILABLE | (len & 0xffff);
}
#endif

//-----------------------------------------------------------------------------
#ifdef ARM7
/*void got_data(u32 len) {
  sprintf((void*)WRAM, "Hi! Got 0x%04x bytes", len);
  REG_IPC_FIFO_TX = COMMS_DATA_AVAILABLE | (20 & 0xffff);
}*/
#endif

#endif // COMMS_H
