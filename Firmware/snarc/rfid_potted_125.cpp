/*
 *   rfid_potted_125.cpp - 125Khz RFID Card reader by POTTEDstuidio
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

#include "rfid_potted_125.h"

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
 
SoftwareSerial RFID_POTTED_125_Serial(RFID_RX_PIN, RFID_TX_PIN);
 
void RFID_POTTED_125::init(void)
{
    RFID_POTTED_125_Serial.begin(RFID_BAUD_RATE);
}

// Some of the code here was copied from
// https://github.com/johannrichard/POTTEDRFIDLib
boolean RFID_POTTED_125::read(unsigned long *last_code)
{
  int timeout;   // Provide some way of exiting the while loop if no chars come
  int bytesRead; // Number of bytes read
  char in;       // The current byte in the buffer
  byte chk;        // Checksum (1 byte)

  // Starts with a 0x02 Ends with 0x03
  if(RFID_POTTED_125_Serial.available())
  {
      bytesRead  = 0;
              
      timeout = 0;
      while(!RFID_POTTED_125_Serial.available() && (timeout++ < RFID_TIMEOUT_COUNT)){}
      if(timeout >= RFID_TIMEOUT_COUNT) { return false; }
      
        globalBuffer[bytesRead++] = in;
        
        timeout = 0;
        while(!RFID_POTTED_125_Serial.available() && (timeout++ < RFID_TIMEOUT_COUNT)){}
        if(timeout >= RFID_TIMEOUT_COUNT) { return false; }
      
      // ID completely read
      Serial.println(globalBuffer);
      
      chk         = strtol(globalBuffer, NULL, 16);

      *last_code  = strtol(globalBuffer, NULL, 16); //hex2dec(str_id.substring(4,10));
 
      Serial.println(*last_code);
      Serial.println(chk);
      return true;
      
  }
  return false;
}

void RFID_POTTED_125::clear(void)
{
    while (RFID_POTTED_125_Serial.available() > 0) {
        RFID_POTTED_125_Serial.read();
    }
    
    RFID_POTTED_125_Serial.flush(); 
}

RFID_POTTED_125 RFIDPOTTED125;
