/*
 *   rfid_raw_125.cpp - 125Khz RFID Card reader by RAWstuidio
 *          (Electronic brick version) though this may work with others
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
 
 /******************************************************************************
 * Includes
 ******************************************************************************/

#include "rfid_raw_125.h"

/******************************************************************************
 * Definitions
 ******************************************************************************/
#define RFID_TIMEOUT_COUNT 20 // 9 seems to be the min, lets double it to be safe..

/******************************************************************************
 * Constructors
 ******************************************************************************/

/******************************************************************************
 * User API
 ******************************************************************************/
 
SoftwareSerial RFID_RAW_125_Serial(RFID_RX_PIN, RFID_TX_PIN);
 
void RFID_RAW_125::init(void)
{
    RFID_RAW_125_Serial.begin(RFID_BAUD_RATE);
}


bool RFID_RAW_125::available(void)
{
    if ( RFID_RAW_125_Serial.available() > 0 ) return true ;
    return false;
}

// Let's read in the tag.
extern unsigned long last_code; // that we put return value into
boolean RFID_RAW_125::read()
{
  //int timeout;   // Provide some way of exiting the while loop if no chars come
  int bytesRead; // Number of bytes read
  //byte chk;        // Checksum (1 byte)
  uint8_t tagBytes[12];
  delay(10);
  static uint32_t cardId = 0;
  static uint8_t checksum = 0;

  //static int testiter = 0; 

  // as they are static, we zero them here.
  cardId = 0;
  checksum = 0;
     //Serial.println("read inside 1"); Serial.flush(); //delay(100);
 
  // These readers don't signal, so read it all.
  if(RFID_RAW_125_Serial.available())
  {
     tagBytes[6] = 0; 
    
    if (RFID_RAW_125_Serial.readBytes(tagBytes, 5) == 5)
  {
         Serial.println("read inside 5 bytes"); Serial.flush(); //delay(100);

        // RFID_RAW_125_Serial.clear(); // throws away any remaining bytes in buffer
    for (int i = 0; i < 4; i++)
    {
      //checksum ^= tagBytes[i];
      cardId = cardId << 8 | tagBytes[i];
      Serial.println(tagBytes[i],HEX);
    }
    if (1) //(checksum == tagBytes[4])
    {
           //Serial.println("read inside checksum "); Serial.flush(); //delay(100);
           Serial.println(F("Dec Tag:"));
            Serial.println(cardId);
            
      last_code = cardId;
      //last_code = cardId+testiter; // TESTING HACK! same card now can be swiped twice at least

     // testiter++;
      
      //chk = checksum;
      return true;
    }
  }  
 //     Serial.println(last_code);
 //     Serial.println(chk);

    Serial.println("read inside 3"); Serial.flush(); //delay(100);

  }
  clear(); // drop any other pending data in the input buffer since we just had a good read.
  return false;
}

void RFID_RAW_125::clear(void)
{
    while (RFID_RAW_125_Serial.available() > 0) {
        RFID_RAW_125_Serial.read();
    }
    
    RFID_RAW_125_Serial.flush(); 
}

RFID_RAW_125 RFIDRAW125;
