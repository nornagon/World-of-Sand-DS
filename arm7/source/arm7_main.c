#include <nds.h>
#include <stdlib.h>
#include "comms.h"
#include "majic.h"

//---------------------------------------------------------------------------------
void startSound(int sampleRate, const void* data, u32 bytes, u8 channel, u8 vol,  u8 pan, u8 format) {
//---------------------------------------------------------------------------------
	SCHANNEL_TIMER(channel)  = SOUND_FREQ(sampleRate);
	SCHANNEL_SOURCE(channel) = (u32)data;
	SCHANNEL_LENGTH(channel) = bytes >> 2 ;
	SCHANNEL_CR(channel)     = SCHANNEL_ENABLE | SOUND_ONE_SHOT | SOUND_VOL(vol) | SOUND_PAN(pan) | (format==1?SOUND_8BIT:SOUND_16BIT);
}


//---------------------------------------------------------------------------------
s32 getFreeSoundChannel() {
//---------------------------------------------------------------------------------
	int i;
	for (i=0; i<16; i++) {
		if ( (SCHANNEL_CR(i) & SCHANNEL_ENABLE) == 0 ) return i;
	}
	return -1;
}


//---------------------------------------------------------------------------------
void VblankHandler(void) {
//---------------------------------------------------------------------------------
	static int heartbeat = 0;

	int t1=0, t2=0;
	uint32 temp=0;
	uint8 ct[sizeof(IPC->curtime)];
	u32 i;

	// Update the heartbeat
	heartbeat++;

	// Read the time
	rtcGetTime((uint8 *)ct);
	BCDToInteger((uint8 *)&(ct[1]), 7);

	// Read the temperature
	temp = touchReadTemperature(&t1, &t2);

	// Update the IPC struct
	IPC->heartbeat	= heartbeat;

	for(i=0; i<sizeof(ct); i++) {
		IPC->curtime[i] = ct[i];
	}

	IPC->temperature = temp;
	IPC->tdiode1 = t1;
	IPC->tdiode2 = t2;


	//sound code  :)
	TransferSound *snd = IPC->soundData;
	IPC->soundData = 0;

	if (0 != snd) {

		for (i=0; i<snd->count; i++) {
			s32 chan = getFreeSoundChannel();

			if (chan >= 0) {
				startSound(snd->data[i].rate, snd->data[i].data, snd->data[i].len, chan, snd->data[i].vol, snd->data[i].pan, snd->data[i].format);
			}
		}
	}
}


int vcount;
touchPosition first, tempPos;

void VcountHandler(void) {
  static int lastbut = -1;

  u16 but=0, x=0, y=0, xpx=0, ypx=0, z1=0, z2=0;

  but = REG_KEYXY;

  if (!( (but ^ lastbut) & (1<<6))) {
    tempPos = touchReadXY();
    x = tempPos.x;
    y = tempPos.y;
    xpx = tempPos.px;
    ypx = tempPos.py;
    z1 = tempPos.z1;
    z2 = tempPos.z2;
  } else {
    lastbut = but;
    but |= (1<<6);
  }

  if (vcount == 80) {
    first = tempPos;
  } else {
    if (abs(xpx - first.px) > 10 || abs(ypx - first.py) > 10 ||
        (but & (1<<6))) {
      but |= (1<<6);
      lastbut = but;
    } else {
      IPC->mailBusy = 1;
      IPC->touchX = x;
      IPC->touchY = y;
      IPC->touchXpx = xpx;
      IPC->touchYpx = ypx;
      IPC->touchZ1 = z1;
      IPC->touchZ2 = z2;
      IPC->mailBusy = 0;
    }
  }
  IPC->buttons = but;
  vcount ^= (80 ^ 130);
  SetYtrigger(vcount);
}


u32 calculate() {
  static int counter = 0;
  u32 x,y;
  u32 particount = 0;
  genrand_regen();

  if (counter)
    for (y = (191-130); y > 0; y--)
      for (x = 1; x < 255; x++)
        particount += majic(WRAM, x, y);
  else
    for (y = (191-130); y > 0; y--)
      for (x = 255; x > 1; x--)
        particount += majic(WRAM, x, y);
  counter = !counter;
  return particount;
}


void FifoHandler() {
  u32 msg = REG_IPC_FIFO_RX;

  if ((msg & ~0xffff) == COMMS_DATA_AVAILABLE) {
    REG_IME = IME_ENABLE; // let vcount handlers interrupt calculate()
    u32 particount = calculate();
    REG_IPC_FIFO_TX = COMMS_DATA_AVAILABLE | (particount & 0xffff);
  }
}

//------------------------------------------------------------------------------
int main(int argc, char ** argv) {
//------------------------------------------------------------------------------
  REG_IPC_FIFO_CR = IPC_FIFO_ENABLE | IPC_FIFO_SEND_CLEAR;
	// Reset the clock if needed
	rtcReset();

	//enable sound
	//powerON(POWER_SOUND);
	//SOUND_CR = SOUND_ENABLE | SOUND_VOL(0x7F);
	//IPC->soundData = 0;

	irqInit();
	irqSet(IRQ_VBLANK, VblankHandler);
  SetYtrigger(80);
  vcount = 80;
  irqSet(IRQ_VCOUNT, VcountHandler);
  irqSet(IRQ_FIFO_NOT_EMPTY, FifoHandler);
	irqEnable(IRQ_VBLANK | IRQ_VCOUNT | IRQ_FIFO_NOT_EMPTY);
  REG_IPC_FIFO_CR = IPC_FIFO_ENABLE | IPC_FIFO_RECV_IRQ;

	uint8 ct[sizeof(IPC->curtime)];
	rtcGetTime((uint8 *)ct);
	BCDToInteger((uint8 *)&(ct[1]), 7);
  init_genrand(*((u32*)ct));

  while (1) swiWaitForVBlank();
}


