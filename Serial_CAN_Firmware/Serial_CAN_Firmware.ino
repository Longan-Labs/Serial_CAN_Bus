/************************************************************************************************* 
  Firmware of Serial can bus module
***************************************************************************************************/
#include <SPI.h>
#include <EEPROM.h>
#include "mcp_can.h"
#include "func.h"
#include "firmware_dfs.h"

extern MCP_CAN CAN;

void sysInit()
{
    pinMode(3, OUTPUT);
    initEEPROM();
    
    serial_can.begin(s_rate[EEPROM.read(EEPADDR_SERIAL)]);
    
    while (CAN_OK != CAN.begin(EEPROM.read(EEPADDR_CANRATE)))    // init can bus : baudrate = 500k
    {
        Serial.println("CAN BUS Shield init fail");
        Serial.println(" Init CAN BUS Shield again");
        delay(100);
        digitalWrite(3, 1-digitalRead(3));
    }

    /*
     * set mask, set both the mask to 0x3ff
     */
    CAN.init_Mask(0, EEPROM.read(EEPADDR_MASK0), readWord(EEPADDR_MASK0+1));
    CAN.init_Mask(1, EEPROM.read(EEPADDR_MASK1), readWord(EEPADDR_MASK1+1));

    /*
     * set filter, we can receive id from 0x04 ~ 0x09
     */
    CAN.init_Filt(0, EEPROM.read(EEPADDR_FILT0), readWord(EEPADDR_FILT0+1)); 
    CAN.init_Filt(1, EEPROM.read(EEPADDR_FILT1), readWord(EEPADDR_FILT1+1));
    CAN.init_Filt(2, EEPROM.read(EEPADDR_FILT2), readWord(EEPADDR_FILT2+1));
    CAN.init_Filt(3, EEPROM.read(EEPADDR_FILT3), readWord(EEPADDR_FILT3+1));
    CAN.init_Filt(4, EEPROM.read(EEPADDR_FILT4), readWord(EEPADDR_FILT4+1));
    CAN.init_Filt(5, EEPROM.read(EEPADDR_FILT5), readWord(EEPADDR_FILT5+1));
    
    showConfig('C');
    showConfig('M');
    showConfig('F');
    
    serial_can.println("CAN INIT OK");
    digitalWrite(3, HIGH);
}

void setup()
{
    sysInit();
    Serial.println("sys start");
}

void loop()
{
    eventSerialCan();
    setMode();
    dataModeSend();
    dataModeRecv();
}

// END FILE