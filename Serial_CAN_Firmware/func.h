// function

#ifndef __FUNC_H__
#define __FUNC_H__

// EEPROM
extern void initEEPROM();
extern void showConfig(unsigned char c);
extern unsigned long readWord(int addr);
extern void writeWord(int addr, unsigned long dta);

// workding mode

extern void eventSerialCan();
extern void setMode();
extern void dataModeSend();
extern void dataModeRecv();

extern unsigned long s_rate[5];

#endif