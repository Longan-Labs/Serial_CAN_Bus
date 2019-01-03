// 2 working __GMode
// set __GMode and data __GMode
#include <Arduino.h>
#include <EEPROM.h>
#include "mcp_can.h"
#include "firmware_dfs.h"
#include "func.h"

unsigned char __GMode = DATA_MODE;

unsigned char __GSerialLen = 0;
unsigned char __GSerialDta[30];
unsigned char __GSerialGet = 0;

MCP_CAN CAN(9);                                    // Set CS pin

const unsigned long s_rate[5] = {9600, 19200, 38400, 57600, 115200};

void eventSerialCan()
{
    static unsigned long timer_s = millis();
    
    while(serial_can.available())
    {
        __GSerialDta[__GSerialLen++] = serial_can.read();
        if(__GSerialDta[__GSerialLen-1] == '\n')__GSerialGet = 1;
        timer_s = millis();
        //Serial.print(__GSerialDta[__GSerialLen-1]);
    }
    
    if(__GSerialLen == 3 && __GSerialDta[0] == '+' && __GSerialDta[1] == '+' && __GSerialDta[2] == '+')     // SET __GMode
    {
        //serial_can.brush();
        serial_can.println("ENTER INTO SETTING MODE");
        __GMode = SET_MODE;
        
        while(serial_can.available())
        {
            serial_can.read();
        }
        __GSerialLen = 0;
    }
    
    if(__GSerialLen && (millis()-timer_s > 10))
    {
        __GSerialLen = 0;
        __GSerialGet = 0;
    }
   
}

void setMode()
{
    if(__GMode != SET_MODE || __GSerialGet == 0)return;
    __GSerialGet = 0;
    digitalWrite(3, LOW);
    delay(10);
    if((__GSerialLen == 3 || __GSerialLen == 4) &&  __GSerialDta[0] == 'A' && __GSerialDta[1] == 'T')
    {
        __GSerialLen = 0;
        Serial.println("OK");
        digitalWrite(3, HIGH);
        return;
    }
    
    if(__GSerialDta[0] != 'A' || __GSerialDta[1] != 'T' || __GSerialDta[2] != '+')goto ERROR;

    int tmp[3];
       
    switch(__GSerialDta[3])
    {
        
        case 'S':                   // set baud of serial_can, AT+S=N
        
        if(__GSerialDta[5]<'0' || __GSerialDta[5]>'4')goto ERROR;
        
        serial_can.println("OK");
        serial_can.begin(s_rate[__GSerialDta[5]-'0']);
        EEPROM.write(EEPADDR_SERIAL, __GSerialDta[5]-'0');
  
        break;
        
        case 'C':                   // set baudrate of CAN BUS
        
        if(__GSerialDta[5]<'0' || __GSerialDta[5]>'9' || __GSerialDta[6]<'0' ||__GSerialDta[6]>'9')goto ERROR;
        tmp[0] = 10*(__GSerialDta[5] - '0')+__GSerialDta[6]-'0';
        
        if(CAN_OK == CAN.begin(tmp[0]))
        {
            serial_can.println("OK");
        }
        else
        {
            serial_can.println("FAIL");
        }
        
        EEPROM.write(EEPADDR_CANRATE, tmp[0]);
        
        break;
        
        case 'M':                   // set mask AT+M=[N][EXT][NUM]
                                    //          0123456789  AB   20
        if(__GSerialDta[6] < '0' || __GSerialDta[6] > '1' || __GSerialDta[9] < '0' || __GSerialDta[9] > '1')goto ERROR;
        tmp[0] = __GSerialDta[6]-'0';
        tmp[1] = __GSerialDta[9]-'0';
        
        if(__GSerialLen < 21 || __GSerialDta[20] != ']')goto ERROR;

        {
            unsigned long __tmp = 0;
            
            for(int i=0; i<8; i++)
            {
                __tmp <<= 4;
                if(__GSerialDta[12+i]>='0' && __GSerialDta[12+i]<='9')
                {
                    __tmp += __GSerialDta[12+i]-'0';
                }
                else if(__GSerialDta[12+i]>='A' && __GSerialDta[12+i]<='F')
                {
                    __tmp += 10+__GSerialDta[12+i]-'A';
                }
                else goto ERROR;
                
            }
            
            Serial.print("cmd: set mask - 0x");
            Serial.print(tmp[0]);
            Serial.print('\t');
            Serial.print(tmp[1]);
            Serial.print('\t');
            Serial.println(__tmp, HEX);
            
            EEPROM.write(EEPADDR_MASK0+10*tmp[0], tmp[1]);
            writeWord(EEPADDR_MASK0+10*tmp[0]+1, __tmp);
            
            CAN.init_Mask(0, EEPROM.read(EEPADDR_MASK0), readWord(EEPADDR_MASK0+1));
            CAN.init_Mask(1, EEPROM.read(EEPADDR_MASK1), readWord(EEPADDR_MASK1+1));

        }
        
        __GSerialLen = 0;
        Serial.println("OK");
        
        break;
        
        case 'F':                   // set filt
        
        if(__GSerialDta[6] < '0' || __GSerialDta[6] > '5' || __GSerialDta[9] < '0' || __GSerialDta[9] > '1')goto ERROR;
        tmp[0] = __GSerialDta[6]-'0';
        tmp[1] = __GSerialDta[9]-'0';
        
        if(__GSerialLen < 21 || __GSerialDta[20] != ']')goto ERROR;
        
        {
            unsigned long __tmp = 0;
            
            for(int i=0; i<8; i++)
            {
                __tmp <<= 4;
                
                if(__GSerialDta[0x0c+i]>='0' && __GSerialDta[0x0c+i]<='9')
                {
                    __tmp += __GSerialDta[0x0c+i]-'0';
                }
                else if(__GSerialDta[0x0c+i]>='A' && __GSerialDta[0x0c+i]<='F')
                {
                    __tmp += 10+__GSerialDta[0x0c+i]-'A';
                }
                else goto ERROR;
            }
            
            
            EEPROM.write(EEPADDR_FILT0+10*tmp[0], tmp[1]);
            writeWord(EEPADDR_FILT0+10*tmp[0]+1, __tmp);
            
            CAN.init_Filt(0, EEPROM.read(EEPADDR_FILT0), readWord(EEPADDR_FILT0+1)); 
            CAN.init_Filt(1, EEPROM.read(EEPADDR_FILT1), readWord(EEPADDR_FILT1+1));
            CAN.init_Filt(2, EEPROM.read(EEPADDR_FILT2), readWord(EEPADDR_FILT2+1));
            CAN.init_Filt(3, EEPROM.read(EEPADDR_FILT3), readWord(EEPADDR_FILT3+1));
            CAN.init_Filt(4, EEPROM.read(EEPADDR_FILT4), readWord(EEPADDR_FILT4+1));
            CAN.init_Filt(5, EEPROM.read(EEPADDR_FILT5), readWord(EEPADDR_FILT5+1));
            
            Serial.print("cmd: set filt - 0x");
            Serial.println(__tmp, HEX);   
        }
        
        __GSerialLen = 0;
        Serial.println("OK");
        
        break;
        
        case 'Q':                   // exit setting mde
        __GSerialLen  = 0;
        __GMode = DATA_MODE;
        Serial.println("OK");
        Serial.println("ENTER DATA MODE");
        break;
        
        default:
        goto ERROR;
    }

    __GSerialLen = 0;
    digitalWrite(3, HIGH);
    return;
    
ERROR:

    serial_can.println("ERROR CMD");
    for(int i=0; i<__GSerialLen; i++)
    {
        Serial.write(__GSerialDta[i]);
    }
    __GSerialLen = 0;
    digitalWrite(3, HIGH);
}

void dataModeSend()
{
    if(DATA_MODE != __GMode)return;
    if(14 != __GSerialLen)return;
    
    unsigned long id  = 0;
    unsigned char ext = 0;
    unsigned char rtr = 0;
    
    for(int i=0; i<4; i++)
    {
        id <<= 8;
        id += __GSerialDta[i];
    }
    
    if(__GSerialDta[4] > 1 || __GSerialDta[5] > 1)return;
    
    ext = __GSerialDta[4];
    rtr = __GSerialDta[5];

    CAN.sendMsgBuf(id, ext, rtr, 8, &__GSerialDta[6]);
    __GSerialLen = 0;
}

void dataModeRecv()
{
    if(DATA_MODE != __GMode)return;
    
    unsigned char len = 0;
    unsigned char buf[8];

    if(CAN_MSGAVAIL == CAN.checkReceive())              // check if data coming
    {
        CAN.readMsgBuf(&len, buf);                      // read data,  len: data length, buf: data buf

        unsigned long id = CAN.getCanId();
        
        Serial.write((id>>24)&0xff);
        Serial.write((id>>16)&0xff);
        Serial.write((id>>8)&0xff);
        Serial.write((id>>0)&0xff);
        
        for(int i=0; i<8; i++)
        Serial.write(buf[i]);
    }
}
// 14, 76
// END FILE