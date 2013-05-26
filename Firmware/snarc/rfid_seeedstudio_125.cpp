/*
 *   rfid_seeedstudio_125.cpp - 125Khz RFID Card reader by seeedstuidio
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

#include "rfid_seeedstudio_125.h"

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
 
SoftwareSerial RFID_SEED_125_Serial(RFID_RX_PIN, RFID_TX_PIN);
 
void RFID_SEEED_125::init(void)
{
    RFID_SEED_125_Serial.begin(RFID_BAUD_RATE);
}

// Some of the code here was copied from
// https://github.com/johannrichard/SeeedRFIDLib
boolean RFID_SEEED_125::read(unsigned long *last_code)
{
  int timeout;   // Provide some way of exiting the while loop if no chars come
  int bytesRead; // Number of bytes read
  char in;       // The current byte in the buffer
  byte chk;        // Checksum (1 byte)

  // Starts with a 0x02 Ends with 0x03
  if(RFID_SEED_125_Serial.available() && (in = RFID_SEED_125_Serial.read()) == 0x02)
  {
      bytesRead  = 0;
              
      timeout = 0;
      while(!RFID_SEED_125_Serial.available() && (timeout++ < RFID_TIMEOUT_COUNT)){}
      if(timeout >= RFID_TIMEOUT_COUNT) { return false; }
      
      
      while((in = RFID_SEED_125_Serial.read()) != 0x03)
      {
        globalBuffer[bytesRead++] = in;
        
        timeout = 0;
        while(!RFID_SEED_125_Serial.available() && (timeout++ < RFID_TIMEOUT_COUNT)){}
        if(timeout >= RFID_TIMEOUT_COUNT) { return false; }
      }
      // ID completely read
      //Serial.println(globalBuffer);
      
      // Read from 10 to 12
      globalBuffer[12] = '\0';
      chk         = strtol(globalBuffer+10, NULL, 16);
      // Read from 4 to 10
      globalBuffer[10] = '\0';
      *last_code  = strtol(globalBuffer+4, NULL, 16); //hex2dec(str_id.substring(4,10));
 
      //mfr = hex2dec(str_id.substring(0,4)); // We dont care about the manufature id..
 
      //Serial.println(*last_code);
      //Serial.println(chk);
      
      // Do checksum calculation
      int checksum = 0;
      for(int i = 0; i < 5; i++) {
        globalBuffer[10] = globalBuffer[i*2];
        globalBuffer[11] = globalBuffer[i*2+1];
        checksum ^= strtol(globalBuffer+10, NULL, 16); //hex2dec(str_id.substring(i*2,(i*2)+2));
      }
      
      if (checksum == chk)
      {
        return true;
      }
//      else
//      {
//        Serial.println("Bad CRC");
//      }
  }
  return false;
}

void RFID_SEEED_125::clear(void)
{
    while (RFID_SEED_125_Serial.available() > 0) {
        RFID_SEED_125_Serial.read();
    }
    
    RFID_SEED_125_Serial.flush(); 
}

RFID_SEEED_125 RFIDseed125;
