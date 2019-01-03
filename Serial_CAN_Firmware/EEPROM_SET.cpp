// eeprom

#include <Arduino.h>
#include <EEPROM.h>
#include "firmware_dfs.h"


void initEEPROM()
{
    // default 9600, 500K, no mask and filter
    if(EEPROM.read(EEPADDR_SET) == 0x55)return;       // set already
    
    EEPROM.write(EEPADDR_SET, 0x55);
    EEPROM.write(EEPADDR_SERIAL, 0);
    EEPROM.write(EEPADDR_CANRATE, 16);
    
    for(int i=10; i<90; i++)
    {
        EEPROM.write(i, 0);
    }
}

unsigned long readWord(int addr)
{
    unsigned long tmp = 0;
    
    for(int i=0; i<4; i++)
    {
        tmp <<= 8;
        tmp += EEPROM.read(addr+i);
    }
    return tmp;
}

void writeWord(int addr, unsigned long dta)
{
    for(int i=3; i>=0; i--)
    {
        EEPROM.write(addr+i, dta&0xff);
        dta >>= 8;
    }
}

void showConfig(unsigned char c)
{
    switch(c)
    {
        case 'M':
        
        Serial.print("Mask0: 0x");
        Serial.println(readWord(EEPADDR_MASK0+1), HEX);
        Serial.print("Mask1: 0x");
        Serial.println(readWord(EEPADDR_MASK1+1), HEX);
        break;
        
        case 'F':
        
        Serial.print("Filt0: 0x");
        Serial.println(readWord(EEPADDR_FILT0+1), HEX);
        Serial.print("Filt1: 0x");
        Serial.println(readWord(EEPADDR_FILT1+1), HEX);
        Serial.print("Filt2: 0x");
        Serial.println(readWord(EEPADDR_FILT2+1), HEX);
        Serial.print("Filt3: 0x");
        Serial.println(readWord(EEPADDR_FILT3+1), HEX);
        Serial.print("Filt4: 0x");
        Serial.println(readWord(EEPADDR_FILT4+1), HEX);
        Serial.print("Filt5: 0x");
        Serial.println(readWord(EEPADDR_FILT5+1), HEX);

        break;
        
        case 'C':       // baud rate
        {
            unsigned int rate[19] = {0, 5, 10, 20, 25, 31, 33, 40, 50, 80, 83, 95, 100, 125, 200, 250, 500, 666, 10000};
            Serial.print("Rate: ");
            Serial.println(rate[EEPROM.read(EEPADDR_CANRATE)]);
        }
        
        break;
        
        default:;
    }
}
