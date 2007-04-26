#include <nds.h>
#include <stdlib.h>
#include "comms.h"
#include "majic.h"

u32 calculate(u16 arm7lines) {
  static int counter = 0;
  u32 x,y;
  u32 particount = 0;
  genrand_regen();

  if (counter)
    for (y = arm7lines; y > 0; y--)
      for (x = 1; x < 255; x++)
        particount += majic(WRAM, x, y);
  else
    for (y = arm7lines; y > 0; y--)
      for (x = 254; x > 0; x--)
        particount += majic(WRAM, x, y);
  // clear edges
  for (y = 0; y < arm7lines; y++) {
    WRAM[y*256+0] = NOTHING;
    WRAM[y*256+255] = NOTHING;
  }
  counter = !counter;
  return particount;
}


void FifoHandler() {
  u32 msg = REG_IPC_FIFO_RX;

  if ((msg & ~0xffff) == COMMS_DATA_AVAILABLE) {
    u32 particount = calculate((msg & 0xffff)/256);
    REG_IPC_FIFO_TX = COMMS_DATA_AVAILABLE | (particount & 0xffff);
  }
}

touchPosition first,tempPos;

//---------------------------------------------------------------------------------
void VcountHandler() {
//---------------------------------------------------------------------------------
	static int lastbut = -1;

	uint16 but=0, x=0, y=0, xpx=0, ypx=0, z1=0, z2=0;

	but = REG_KEYXY;

	if (!( (but ^ lastbut) & (1<<6))) {

		tempPos = touchReadXY();

		if ( tempPos.x == 0 || tempPos.y == 0 ) {
			but |= (1 <<6);
			lastbut = but;
		} else {
			x = tempPos.x;
			y = tempPos.y;
			xpx = tempPos.px;
			ypx = tempPos.py;
			z1 = tempPos.z1;
			z2 = tempPos.z2;
		}

	} else {
		lastbut = but;
		but |= (1 <<6);
	}

	IPC->touchX		= x;
	IPC->touchY		= y;
	IPC->touchXpx	= xpx;
	IPC->touchYpx	= ypx;
	IPC->touchZ1	= z1;
	IPC->touchZ2	= z2;
	IPC->buttons	= but;
}

//---------------------------------------------------------------------------------
int main(int argc, char ** argv) {
//---------------------------------------------------------------------------------
	// Reset the clock if needed
	rtcReset();
  rtcGetTimeAndDate((uint8*)&(IPC->time.rtc.year));

  REG_IPC_FIFO_CR = IPC_FIFO_ENABLE | IPC_FIFO_SEND_CLEAR;

	irqInit();

	SetYtrigger(80);
	irqSet(IRQ_VCOUNT, VcountHandler);
  irqSet(IRQ_FIFO_NOT_EMPTY, FifoHandler);
	irqEnable(IRQ_FIFO_NOT_EMPTY | IRQ_VCOUNT);

  REG_IPC_FIFO_CR = IPC_FIFO_ENABLE | IPC_FIFO_RECV_IRQ;

  init_genrand(*((u32*)IPC->time.curtime));

	IPC->mailBusy = 0;

	// Keep the ARM7 out of main RAM
	while (1) swiWaitForVBlank();
}

