/*
 *   config.h - Setup the board with all the options avaliable 
 *   Copyright (C) 2013 Luke Hovigton. All right reserved.
 *
 *   This program is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
 
#ifndef __CONFIGURE_H__
#define __CONFIGURE_H__

// ------ Define Board ------
//#define SNARC_PLUS
#define NETROL
//#define SNARC

// ------ Set RFID Input ------
//#define RFID_125MHZ_SOFTSERIAL
#define RFID_RAW
#define RFID_RX_PIN 15
#define RFID_TX_PIN 14

// ------ Memory Storage ------
#define USE_EEPROM

// ------ Door Settings ------
// #define DOOR_INVERT_PIN // Invert the pin logic to the output Relay/Mosfet
//#define ENABLE_ESTOP_AS_SAFETY_DEVICE
//#define ENABLE_ESTOP_AS_PWM_COUNTER
#define ENABLE_ESTOP_AS_EGRESS_BUTTON

// ------ Ethernet Protocol ------
#define USE_ETHERNET_HTTP
//#define USE_ETHERNET_SOCKET
//#define USE_ETHERNET_HTTP_SERVER

/* -------------------------------------
 *      END OF USER CONFIG OPTIONS
 * -------------------------------------
 */

#include "IPAddress.h"
#include <inttypes.h>

#define MEMORY_DEVICE_NAME_MAX_LENGTH 12
#define GLOBAL_BUFFER_LEN             40

extern char globalBuffer[GLOBAL_BUFFER_LEN];

// Declare Client info to be stored
struct DeviceInfo {
  IPAddress     ip;        // IP address of this device
  IPAddress     gateway;   // Gateway to the Internet
  IPAddress     subnet;    // Subnet Mask
  IPAddress     server;    // IP address of the authentication server
  uint8_t       mac[6];    // Mac address of this device
  char          deviceName[MEMORY_DEVICE_NAME_MAX_LENGTH]; // Name/Location of this device
  unsigned long int id;        // Device unique id can not exceed 65,536
};

struct RFID_info {
    unsigned long  card;
};

extern unsigned long rfidTag;
extern DeviceInfo mySettings;



// --- Include the interactive serial menu ---
#include "serial_menu.h"
#define MENU SNARC_SERIAL_MENU

// --- Setup LED Defines ---
// Though you may have less colours define them anyway
#define LEDS_RED    1<<0
#define LEDS_BLUE   1<<1
#define LEDS_GREEN  1<<2
#define LEDS_WHITE  1<<3
#define LEDS_ALL    (LEDS_RED | LEDS_BLUE | LEDS_GREEN | LEDS_WHITE)


//#define W5100_ETHERNET_SHIELD // Arduino Ethenret Shield and Compatibles ...
//#define W5200_ETHERNET_SHIELD // WIZ820io, W5200 Ethernet Shield
//#define W5500_ETHERNET_SHIELD   // WIZ550io, ioShield series of WIZnet
// --- SNARC_PLUS Board Config ---
#ifdef SNARC_PLUS
    #define W5200_ETHERNET_SHIELD // WIZ820io, W5200 Ethernet Shield

    #define LEDS         generic_leds

    #define DOOR_PIN           8
    #define ETHERNET_CS        4
    #define ETHERNET_RESET_PIN 7

    #define INT_ETHERNET       0
    #define INT_USER           1

    #define LED_PIN_RED    10
    #define LED_PIN_BLUE   9
    #define LED_PIN_GREEN  6

    #define LED_PINS    {LED_PIN_RED, LED_PIN_BLUE, LED_PIN_GREEN}
    #define LED_DEFINED 3

    #include "leds_generic.h"
#elif defined NETROL
    #define W5500_ETHERNET_SHIELD   // WIZ550io, ioShield series of WIZnet

    #define LEDS         generic_leds

    #define DOOR_PIN           8
    #define ETHERNET_CS        D4
    #define ETHERNET_RESET_PIN 7

    #define INT_ETHERNET       0
    #define INT_USER           1

    #define LED_PIN_RED    10
    #define LED_PIN_BLUE   9
    #define LED_PIN_GREEN  6

    #define LED_PIN_WHITE  5
    #define LED_PINS    {LED_PIN_RED, LED_PIN_BLUE, LED_PIN_GREEN, LED_PIN_WHITE}
    #define LED_DEFINED 4

    #include "leds_generic.h"
#elif defined SNARC
    #define W5200_ETHERNET_SHIELD // WIZ820io, W5200 Ethernet Shield

    #define LEDS         generic_leds

    #define DOOR_PIN            5
    #define ETHERNET_CS         10
    #define ETHERNET_RESET_PIN  8
    #define WIZRESET ETHERNET_RESET_PIN // alternate name for same pin
    
    #define INT_ETHERNET       0
    #define INT_USER           1
    
    #define LED_PIN_RED    17
    #define LED_PIN_YELLOW 18
    #define LED_PIN_GREEN  19
    
    #define LED_PINS    {LED_PIN_RED, LED_PIN_YELLOW, LED_PIN_GREEN}
    #define LED_DEFINED 3

    #include "leds_generic.h"
#else
    #error No Board Defined!
#endif

// --- Include ethernet chip libary --
#include "ethlib_Ethernet.h"

// --- Include Door code ---
#include "door_actuator.h"

// --- Include RFID driver ---
#ifdef RFID_125MHZ_SOFTSERIAL
    #define  RFID_SEEED_RX RFID_RX_PIN
    #define  RFID_SEEED_TX RFID_TX_PIN
    #include "rfid_seeedstudio_125.h"
    #define  RFID RFIDSEED125
#elif defined RFID_POTTED
    #define  RFID_SEEED_RX RFID_RX_PIN
    #define  RFID_SEEED_TX RFID_TX_PIN
    #include "rfid_potted_125.h"
    #define RFID RFIDPOTTED125
#elif defined RFID_RAW
    #define  RFID_SEEED_RX RFID_RX_PIN
    #define  RFID_SEEED_TX RFID_TX_PIN
    #include "rfid_raw_125.h"
    #define RFID RFIDRAW125
#else
    #error DEFINE A RFID READER
#endif

    //--- Specify storage ---
#ifdef USE_EEPROM
    #include "memory_eeprom.h"
    #define MEMORY eepromMemory
#elif defined USE_FLASH_AT45DB
    #define MEMORY_FLASH_AT45DB_CS 10
    #include "memory_flash_at45db.h"
    #define MEMORY MEMORY_FLASH_AT45DB
#else
    #error NO MEMORY DEFINED
#endif

// --- Specify Ethernet Protocol ---
#ifdef USE_ETHERNET_HTTP
    #include "ethernet_http.h"
    #define ETHERNET ethernetHttp
    #define NETWORKCHECKER ethernetWiznetChecker
#elif defined USE_ETHERNET_SOCKET
    #include "ethernet_socket.h"
    #define ETHERNET ethernetSocket
    #define NETWORKCHECKER ethernetSocketChecker
//#elif defined USE_ETHERNET_HTTP_SERVER

#else
    #error NO ETHERNET DEFINED
#endif

#if defined ENABLE_ESTOP_AS_SAFETY_DEVICE && defined ENABLE_ESTOP_AS_EGRESS_BUTTON
  #error You Cant use Estop for both "Safty" and "Egress"
#endif

#if !defined ENABLE_ESTOP_AS_SAFETY_DEVICE && !defined ENABLE_ESTOP_AS_EGRESS_BUTTON
  #error You should define either "Safty" and "Egress"
#endif

// Workaround for http://gcc.gnu.org/bugzilla/show_bug.cgi?id=34734
#ifdef PROGMEM
#undef PROGMEM
#define PROGMEM __attribute__((section(".progmem.data")))
#endif

#endif /* __CONFIGURE_H__ */
