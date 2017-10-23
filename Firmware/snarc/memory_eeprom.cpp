  /*
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

#include "memory_eeprom.h"

/******************************************************************************
 * Definitions
 ******************************************************************************/

/******************************************************************************
 * Constructors
 ******************************************************************************/

/******************************************************************************
 * User API
 ******************************************************************************/


// saves eeprom wear.
void myWrite (int address, int val)
{
  if (val != EEPROM.read(address)) {
    EEPROM.write(address, val);

    // read-after-write to verify it really worked:
    if (val != EEPROM.read(address) ) { 
      Serial.println("EEPROM read-after-write failed"); 
    }
  }
}

 
void MEMORY_EEPROM::init(void)
{
    defaultCalled = false;
 
        //Serial.print(MEMORY_HEADER_LEN);
        Serial.print(F(" space avaliable "));
        Serial.print(sizeof(DeviceInfo));
        Serial.println(F(" taken."));
}

bool MEMORY_EEPROM::storeNetworkInfo(DeviceInfo *device)
{
    unsigned int i;
    for(i=0;i<sizeof(DeviceInfo);i++)
    {
       myWrite(i, ((byte*) device)[i]);
    }
    return true;
}

bool MEMORY_EEPROM::getNetworkInfo(DeviceInfo *device)
{
    unsigned int i;
    boolean valid = false;
    
    // Read EEPROM data
    for(i=0;i<sizeof(DeviceInfo);i++)
    {
      ((byte*) device)[i] = EEPROM.read(i);
    }

    // Ip Default
    if (device->ip == IPAddress(0xFFFFFFFF))
    {
       defaultOutput(true);
       Serial.println(F("Using Default IP - 10.0.1.228"));
       device->ip = IPAddress(10,0,1,228);
    }

    // Gateway Default
    if (device->gateway == IPAddress(0xFFFFFFFF))
    {
       defaultOutput(true);
       Serial.println(F("Using Default Gateway - 10.0.1.254"));
       device->gateway = IPAddress(10,0,1,254);
    }
    
    // Subnet Default
    if (device->subnet == IPAddress(0xFFFFFFFF))
    {
       defaultOutput(true);
       Serial.println(F("Using Default Subnet IP - 255.255.254.0"));
       device->subnet = IPAddress(255,255,254,0);
    }
    
    // Server Default
    if (device->server == IPAddress(0xFFFFFFFF))
    {
       defaultOutput(true);
       Serial.println(F("Using Default Server - 10.0.1.253"));
       device->server = IPAddress(10,0,1,253);
    }
    
    // Mac Address Default
    if(device->mac[0] != 0x02 || device->mac[1] != 0x08 || device->mac[2] != 0xDC)
    {
       defaultOutput(true);
       Serial.println(F("Using Default Mac Address - 02:08:DC:EF:EF:BF"));
       // Static OUI (Organizationally Unique Identifier)
       device->mac[0] = 0x02;
       device->mac[1] = 0x08;
       device->mac[2] = 0xDC;
       // NIC (Network Interface Controller)
       device->mac[3] = 0xEF;
       device->mac[4] = 0xEF;
       device->mac[5] = 0xBF;
    }
    
    // Device Name Default
    valid = false;
    for(i=0;i<MEMORY_DEVICE_NAME_MAX_LENGTH && !valid;i++)
    {
        if((byte) device->deviceName[i] != 0xff)
        {
            valid = true;
        }
    }
    if(!valid)
    {
       defaultOutput(true);
       Serial.println(F("Using Default Name - default"));
       device->deviceName[0] = 'D';
       device->deviceName[1] = 'e';
       device->deviceName[2] = 'f';
       device->deviceName[3] = 'a';
       device->deviceName[4] = 'u';
       device->deviceName[5] = 'l';
       device->deviceName[6] = 't';
       device->deviceName[7] = '\0';
       //device->deviceName[8] = '\0'; // End char
    }
    
    
    // Device Id Default
    if (device->id == 0xFFFF)
    {
       defaultOutput(true);
       Serial.println(F("Using Default Id - 42"));
       device->id = 42; // not zero.
    }
    
    defaultOutput(false);
    return true;
}

void MEMORY_EEPROM::defaultOutput(boolean isStart)
{
    if(isStart && !defaultCalled)
    {
       defaultCalled = true;
       Serial.println(F("-- No config found in memory using defaults --"));
    }
    else if(!isStart && defaultCalled)
    {
       Serial.println(F("----------------------------------------------"));
    }
}

// as a side-effect of 'accessAllowed' being run ( since we can't retunr more than one value, it also updates this var with the next-unused-offset in eeprom)

unsigned long next_unused_offset = 0;

// Find if card is in memory, if not add it.
bool MEMORY_EEPROM::storeAccess(RFID_info access)
{
    //unsigned int i,j;
    //RFID_info entry;

    // if it's in the EEPROM cache already... 
    if ( accessAllowed(access.card)  == true ) { 
      return true; // nothing more to do here.
    }

    if ( next_unused_offset+8 > E2END ) { Serial.println(F("EEPROM FULL, NOT SAVED, SORRY"));  return false; } 

    // next_unused_offset is now where can place next tag:

    Serial.print(F("next_unused_offset:")); Serial.println(next_unused_offset);
    myWrite(next_unused_offset, (byte)access.bytes.byte1);
    myWrite(next_unused_offset+1,  (byte)access.bytes.byte2);
    myWrite(next_unused_offset+2,  (byte)access.bytes.byte3);
    myWrite(next_unused_offset+3,  (byte)access.bytes.byte4);

    next_unused_offset+=4; 

    // and just-in-case eeprom isn't clear after this one, we'll write FF's after: 
    Serial.print(F("next_unused_offset:")); Serial.println(next_unused_offset);
    myWrite(next_unused_offset, 255);
    myWrite(next_unused_offset+1,  255);
    myWrite(next_unused_offset+2,  255);
    myWrite(next_unused_offset+3,  255);

    return true;

}

// Returns true if card is valid and hasn't expired
 
bool MEMORY_EEPROM::accessAllowed(unsigned long rfid)
{
    unsigned int i,j;
    RFID_info entry; // tmp 

    // the size of the eeprom , or the 'last eprom address' : 
    Serial.println(E2END);
    Serial.println(MEMORY_RFID_LENGTH);

    // MEMORY_HEADER_LEN = 100 ( we skip forst 100 bytes that are used for ethernet/ip storage
    // MEMORY_RFID_LENGTH = sizeof(RFID_info) , currently a struct with just  an 'unsigned long' in it. :-)  //  this is how we 'hop' through the eeprom looking at next offset/s
    
    for(i=MEMORY_HEADER_LEN; i <= E2END ;i+=MEMORY_RFID_LENGTH)
    {
        //Serial.print(F("LINE:")); Serial.print(i-100); Serial.print(F("  "));

        if ( sizeof(RFID_info) != 4 ) {   Serial.println(F("ERROR ERROR ERROR ERROR ERROR sizeof(RFID_info) != 4 "));  } 
 
        //((byte*) &entry)[j] = EEPROM.read(i+j);  // crashes on tag read with empty eeprom, as we try to read too much..? 
        entry.bytes.byte1 = EEPROM.read(i);
        entry.bytes.byte2 = EEPROM.read(i+1);
        entry.bytes.byte3 = EEPROM.read(i+2);
        entry.bytes.byte4 = EEPROM.read(i+3);

        // from eeprom as bytes 
//        Serial.print(EEPROM.read(i)); Serial.print(F("/"));
//        Serial.print(EEPROM.read(i+1));  Serial.print(F("/"));
//        Serial.print(EEPROM.read(i+2)); Serial.print(F("/"));
//        Serial.print(EEPROM.read(i+3));
//        Serial.print(F(" -> "));
//        // from eeprom as 'long'
//        Serial.println((unsigned long)entry.card);
//        
        if(entry.card == 0xFFFFFFFF)
        {
            Serial.print(F("NOT FOUND")); 
            next_unused_offset = i; // we "return" this result via a global, because its easy. 
            return false;
        }
        if(entry.card == rfid)
        {
           Serial.print(F("FOUND OK:"));
           Serial.println(rfid); 
           return true;
        }

    }
    
    return false;
}

void MEMORY_EEPROM::test_and_wipe_eeprom() { 
  int i; 
  byte byte1;
  bool failure = false;
   for(i=MEMORY_HEADER_LEN; i <= E2END ;i+=1)
    {
      //byte1 = EEPROM.read(i);
      myWrite(i,i%255); // yes, we write something derived from the value 'i' to offset 'i' as test data.
      byte1 = EEPROM.read(i);
      if ( (int) byte1 != i%255 ) {  failure = true;  Serial.print(i); Serial.print(F(" ")); Serial.print(byte1); Serial.println();} 
    }
    if ( failure) Serial.print(F(" ERROR ERROR ERROR  test_and_wipe_eeprom FAILED FAILED FAILED - UNRELIABLE EEPROM! "));  
}

//// Set the timestamp to zero
//bool MEMORY_EEPROM::expireAccess()
//{
//    unsigned int i;
//    
//    for(i=MEMORY_HEADER_LEN;i <= E2END;i++)
//    {
//        myWrite(i, 0xff);
//    }
//    return true; 
//}

// for debugging
void MEMORY_EEPROM::dumphead(void){ 

    unsigned int i,j;
    //RFID_info entry;
    
    for(i=MEMORY_HEADER_LEN; i <= 140 ;i+=MEMORY_RFID_LENGTH)   // only ~50 bytes for test 
    {
      //Serial.print("LINE: "); Serial.print(i); Serial.print(" | ");  
        byte b1  = EEPROM.read(i);
        byte b2  = EEPROM.read(i+1);
        byte b3  = EEPROM.read(i+2);
        byte b4  = EEPROM.read(i+3);
//        Serial.print( b1); Serial.print(" "); 
//        Serial.print( b2); Serial.print(" "); 
//        Serial.print( b3); Serial.print(" "); 
//        Serial.print( b4); Serial.println(" ");     
     }
  
}

// three reads from eeprom are the same..? 
byte reliable_read(int q) { 
  byte bA  = EEPROM.read(q);
  //delay(5); 
  byte bB  = EEPROM.read(q);
  byte bC  = EEPROM.read(q);
  if (( bA == bB ) && ( bB == bC )) { return bA; } 
  // if three reads don't match, recurse till they do. :-) 
  return reliable_read(q); 
}

// Print access list and timestamps
void MEMORY_EEPROM::printAccessList(void)
{   
    //unsigned int i,j;
    RFID_info entry;
    
    for(int z=MEMORY_HEADER_LEN; z <= E2END ;z+=MEMORY_RFID_LENGTH)
    {
        Serial.print(F("z: ")); Serial.print( z);  Serial.print(F(" "));
  
        if ( MEMORY_RFID_LENGTH != 4 ) { Serial.println(F("ERROR ERROR ERROR MEMORY_RFID_LENGTH  WRONG! MUST BE 4 BYTES ! ")); }  

        byte b1  = reliable_read(z); //delay(50);
        byte b2  = reliable_read(z+1); //delay(50);
        byte b3  = reliable_read(z+2);//delay(50);
        byte b4  = reliable_read(z+3);//delay(50);

        // delay(50); // important to allow the EEPROM read/s to finish, or otherwise we have been known to get trash from eeprom.? 
        Serial.print("b1:");  Serial.print( b1,DEC); 
        Serial.print(" b2:"); Serial.print( b2,DEC); 
        Serial.print(" b3:"); Serial.print( b3,DEC); 
        Serial.print(" b4:"); Serial.println( b4,DEC); 
        
        entry.bytes.byte1 = b1; //delay(50);
        entry.bytes.byte2 = b2;//delay(50);
        entry.bytes.byte3 = b3;//delay(50);
        entry.bytes.byte4 = b4;//delay(50);

      //  delay(50);

        // from eeprom as bytes 
        //Serial.print(EEPROM.read(i)); Serial.print(F("/"));
        //Serial.print(EEPROM.read(i+1));  Serial.print(F("/"));
        //Serial.print(EEPROM.read(i+2)); Serial.print(F("/"));
        //Serial.print(EEPROM.read(i+3));

        //Serial.print(F(" -> "));

        //Serial.print((uint8_t)entry.bytes.byte1); Serial.print(F("!"));
        //Serial.print((uint8_t)entry.bytes.byte2);Serial.print(F("!"));
        //Serial.print((uint8_t)entry.bytes.byte3);Serial.print(F("!"));
        //Serial.print((uint8_t)entry.bytes.byte4);

        
        //Serial.print(F(" -< "));
        // from eeprom as 'long'
        //Serial.println((unsigned long)entry.card);

        if ( (entry.bytes.byte1 == 255) && (entry.bytes.byte2 == 255) && (entry.bytes.byte3 == 255) && (entry.bytes.byte4 == 255) ) { 
           return;
        }

        if(entry.card == 0xFFFFFFFF)
        { 
          return;  // 0xFFFFFFFF signals unused memory from this point forward.
        }
        Serial.print(F("ACCESS ITEM:"));
        Serial.println((unsigned long)entry.card);
    }
}

bool MEMORY_EEPROM::erase_rfid_tags(void)
{
    for (int i = MEMORY_HEADER_LEN; i <= E2END; i++) {
      myWrite(i, 0xff);
    }
    
    return true;   
}
bool MEMORY_EEPROM::erase_network_settings(void)
{
    for (int i = 0; i < MEMORY_HEADER_LEN; i++) {
      myWrite(i, 0xff);
    }
    
    return true;   
}

MEMORY_EEPROM eepromMemory;
