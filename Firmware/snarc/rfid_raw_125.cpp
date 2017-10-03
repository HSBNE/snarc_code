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

// Let's read in the tag.
boolean RFID_RAW_125::read(unsigned long *last_code)
{
  int timeout;   // Provide some way of exiting the while loop if no chars come
  int bytesRead; // Number of bytes read
  byte chk;        // Checksum (1 byte)
  uint8_t tagBytes[6];
  delay(10);
  // These readers don't signal, so read it all.
  if(RFID_RAW_125_Serial.available())
  {
     tagBytes[6] = 0; 
    
    if (RFID_RAW_125_Serial.readBytes(tagBytes, 5) == 5)
  {
    uint8_t checksum = 0;
    uint32_t cardId = 0;
    for (int i = 0; i < 4; i++)
    {
      checksum ^= tagBytes[i];
      cardId = cardId << 8 | tagBytes[i];
      Serial.println(tagBytes[i],HEX);
    }
    if (checksum == tagBytes[4])
    {
           Serial.println("Dec Tag:");
            Serial.println(cardId);
      *last_code = cardId;
      chk = checksum;
      return true;
    }
  }  
 //     Serial.println(*last_code);
 //     Serial.println(chk);
            
  }
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
