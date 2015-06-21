#include "config.h"
#include <SoftwareSerial.h>
#include <SPI.h>
#include <EEPROM.h>
#include "ST7565.h"

#ifdef USE_LCD
#include "lcd_st7565.h"
#include "TimerOne.h"
#endif

/*
 *   Simple NetworkAble RFID Controller firmware, for SNARC, SNARC+, NetTroll and Arduino+Ethernet comptible hardware.
 *   Copyright (C) 2013 Luke Hovigton & David Bussenschutt. All right reserved.
 */
 
// TODO: 
// socket library
// http web interface ( crashes sometimes ) 
// watchdog reset
// password on web interface
// "get current full list from server" menu option not implemented yet
// push full list of auth data from server ( to http interface ) 
// key revocation is tested and works ( by setting timestamp to zero ) 
// make the hardware "zero configuration" with DHCP and some sort of registration process.


DeviceInfo mySettings;
unsigned long rfidTag;
char globalBuffer[GLOBAL_BUFFER_LEN];
#ifdef USE_LCD
uint32_t timeUpdate = 0;
#endif

void userInterupt();
int freeRam(void);

void setup()
{    
    LEDS.init();
    LEDS.on(LEDS_ALL);
#ifdef USE_LCD
    LCD.init();
#endif
    MENU.init(19200); // Set the TX/RX pins to 19200
    //MENU.display(); // force enter programming mode always.
    RFID.init();
    MEMORY.init();
    MEMORY.getNetworkInfo(&mySettings);
    //MEMORY.storeNetworkInfo(&mySettings);
    Serial.print(F("Device name: "));
    Serial.print(mySettings.deviceName);
    Serial.print(" id:");
    Serial.println(mySettings.id);
#ifdef ETHERNET    
    ETHERNET.init(mySettings.mac, mySettings.ip, mySettings.gateway, mySettings.subnet, mySettings.server);
    NETWORKCHECKER.init();
#endif    
    DOOR.init();
#ifdef USE_LCD
    Timer1.initialize(100000); // initialize timer1, and set a 1/2 second period
    Timer1.attachInterrupt(timerInterupt);
#endif
    attachInterrupt(INT_USER, userInterupt, LOW);
    Serial.print(freeRam());

    delay(200);
#ifdef USE_LCD
    LCD.start();
#endif
}

void loop()
{
    LEDS.toggle(LEDS_WHITE, 2000);
    MENU.check();
    
    if(RFID.read(&rfidTag))
    {
        MEMORY.getNetworkInfo(&mySettings);
        Serial.print(F("RFID Tag:"));
        Serial.println(rfidTag);
        LEDS.off(LEDS_BLUE);
        
        if(MEMORY.accessAllowed(&rfidTag)) // is tag in local EEPROM? 
        {
            LEDS.on(LEDS_GREEN);
            DOOR.unlockDoor(2000, &rfidTag, &mySettings.id, mySettings.deviceName); // open door for 2 seconds and log to HTTP remote
            LEDS.off(LEDS_GREEN);
        }
        else 
#ifdef ETHERNET        
        if (ETHERNET.check_tag(&rfidTag, &mySettings.id, mySettings.deviceName) > 0) // unknown key, check what remote server has to say ( server logs it) ? 
        {    
             LEDS.on(LEDS_GREEN | LEDS_RED);
             DOOR.unlockDoor(2000); // open door for 2 seconds , no logging
             
             // Record Card for next time
             RFID_info newCard = {rfidTag};
             MEMORY.storeAccess(&newCard);
             LEDS.off(LEDS_GREEN | LEDS_RED);
        }
        else
#endif        
        {
            LEDS.blink(LEDS_RED);
        }
    }
#ifdef USE_LCD
    LCD.updateCounter(timeUpdate);
#endif    
#ifdef ETHERNET     
    ETHERNET.listen();   // local http server handler.
    NETWORKCHECKER.listen();
#endif    
    DOOR.locktimeout();
}

void userInterupt()
{
#ifdef ENABLE_ESTOP_AS_SAFETY_DEVICE
    DOOR.lock();
#endif
#ifdef ENABLE_ESTOP_AS_EGRESS_BUTTON
    DOOR.unlockDoor(2000); // open door for 2 seconds
#endif
}
#ifdef USE_LCD
void timerInterupt()
{
    // Do something
    timeUpdate++;
}
#endif
// this handy function will return the number of bytes currently free in RAM, great for debugging!
int freeRam(void)
{
    extern int  __bss_end;
    extern int  *__brkval;
    int free_memory;
    if((int)__brkval == 0) {
        free_memory = ((int)&free_memory) - ((int)&__bss_end);
    }
    else {
        free_memory = ((int)&free_memory) - ((int)__brkval);
    }
    return free_memory;
}
