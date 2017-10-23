#include "config.h"
#include <SoftwareSerial.h>
#include <SPI.h>
#include <EEPROM.h>

// uncomment this whole block to compile with LCD support and TimerOne 
//#define USE_LCD
//#ifdef USE_LCD
//#include "ST7565.h"
//#include "lcd_st7565.h"
//#include "TimerOne.h"
//#endif

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

unsigned long last_code = 0;

char globalBuffer[GLOBAL_BUFFER_LEN];
#ifdef USE_LCD
uint32_t timeUpdate = 0;
#endif

void userInterupt();
//int freeRam(void);


int freeMemory();

  #include <Arduino.h>

  extern unsigned int __heap_start;
extern void *__brkval;

/*
 * The free list structure as maintained by the
 * avr-libc memory allocation routines.
 */
struct __freelist {
  size_t sz;
  struct __freelist *nx;
};

/* The head of the free list structure */
extern struct __freelist *__flp;



/* Calculates the size of the free list */
int freeListSize() {
  struct __freelist* current;
  int total = 0;
  for (current = __flp; current; current = current->nx) {
    total += 2; /* Add two bytes for the memory block's header  */
    total += (int) current->sz;
  }
  return total;
}


int freeMemory() {
  int free_memory;
  if ((int)__brkval == 0) {
    free_memory = ((int)&free_memory) - ((int)&__heap_start);
  } else {
    free_memory = ((int)&free_memory) - ((int)__brkval);
    free_memory += freeListSize();
  }
  return free_memory;
}

void setup()
{    
    LEDS.init();
    LEDS.on(LEDS_ALL);
#ifdef USE_LCD
    LCD.init();
#endif
    MENU.init(19200); // Set the TX/RX pins to 19200

    Serial.print(F("freeMemory()="));
    Serial.println(freeMemory());
    Serial.print(F("freeRAM()="));
    Serial.println(freeRam());

    RFID.init();
    MEMORY.init();
    MEMORY.getNetworkInfo(&mySettings);
    Serial.print(F("Device name: "));
    Serial.print(mySettings.deviceName);
    Serial.print(F(" id:"));
    Serial.println(mySettings.id);
    //Serial.print(MEMORY_HEADER_LEN);
    Serial.print(F(" space avaliable "));
    Serial.print(sizeof(DeviceInfo));
    Serial.println(F(" taken."));
    ETHERNET.init(mySettings.mac, mySettings.ip, mySettings.gateway, mySettings.subnet, mySettings.server);
    NETWORKCHECKER.init();
    DOOR.init();
#ifdef USE_LCD
    Timer1.initialize(100000); // initialize timer1, and set a 1/2 second period
    Timer1.attachInterrupt(timerInterupt);
#endif
    attachInterrupt(INT_USER, userInterupt, LOW);

    delay(200);
#ifdef USE_LCD
    LCD.start();
#endif
}

void loop()
{
    LEDS.toggle(LEDS_WHITE, 2000);
    //Serial.println("loopcheck"); Serial.flush(); //delay(100);
    MENU.check();
    //Serial.println("after menu check"); Serial.flush(); //delay(100);

    bool goodread =  RFID.read();
    //rfidTag =  last_code;
    if( goodread )
    {
         Serial.println("read returned ok1"); Serial.flush(); //delay(100);
 
        MEMORY.getNetworkInfo(&mySettings);
        Serial.print(F("RFID Tag:"));
        //Serial.println("read returned ok2"); Serial.flush(); //delay(100);
        Serial.println(last_code);
        //Serial.println("read returned ok3"); Serial.flush(); //delay(100);
        LEDS.off(LEDS_BLUE);
        //Serial.println("read returned ok4"); Serial.flush(); //delay(100);
        bool cached = MEMORY.accessAllowed(last_code);
        Serial.println(cached?"OKcached":"NOTcached"); Serial.flush(); //delay(100);
        if(cached) // is tag in local EEPROM? 
        {
        Serial.println("MEMORY.accessAllowed"); Serial.flush(); //delay(100);
            LEDS.on(LEDS_GREEN);
            DOOR.unlockDoor(2000, &last_code, &mySettings.id, mySettings.deviceName); // open door for 2 seconds and log to HTTP remote
            LEDS.off(LEDS_GREEN);
        }
        else if (ETHERNET.check_tag(&last_code, &mySettings.id, mySettings.deviceName) > 0) // unknown key, check what remote server has to say ( server logs it) ? 
        {    
        Serial.println("MEMORY.denied AND ETHERNET.check_tag OK "); Serial.flush(); //delay(100);
             LEDS.on(LEDS_GREEN | LEDS_RED);
             DOOR.unlockDoor(2000); // open door for 2 seconds , no logging
             
             // Record Card for next time
             RFID_info newCard = {last_code};
             MEMORY.storeAccess(newCard);
             LEDS.off(LEDS_GREEN | LEDS_RED);
        }
        else
        {
        Serial.println("neither, blinck RED"); Serial.flush(); //delay(100);
            LEDS.blink(LEDS_RED);
        }
    }
    //Serial.println("E listen"); Serial.flush(); //delay(100);
    ETHERNET.listen();   // local http server handler.
//#ifdef USE_LCD
//    LCD.updateCounter(timeUpdate);
//#endif
    //Serial.println("N listen"); Serial.flush(); //delay(100);
    NETWORKCHECKER.listen();
    //Serial.println("locktimeout"); Serial.flush(); //delay(100);
    DOOR.locktimeout();
}

void userInterupt()
{
#ifdef ENABLE_ESTOP_AS_SAFETY_DEVICE
    DOOR.lock();
#endif
#ifdef ENABLE_ESTOP_AS_EGRESS_BUTTON
    DOOR.unlockDoor(5000); // open door for 2 seconds
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
