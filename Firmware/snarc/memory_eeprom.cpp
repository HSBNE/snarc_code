/*
 *   rfid_seeedstudio_125.cpp - 125Khz RFID Card reader by seeedstuidio (Electronic brick version) though this may work with others
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
// be sure to start in the EEPROM at MEMORY_HEADER_LEN ( 50) and move in steps of MEMORY_RFID_LENGTH(11) from there.
// this is for internal use inside the EEPROM library only. 
bool MEMORY_EEPROM::_write_to_eeprom_at_fixed_offset(RFID_info *entry, int offset){
    int j=0;
    for(j=sizeof(entry->card);j<MEMORY_RFID_LENGTH;j++)
    {
        EEPROM.write(offset+j, ((byte*) entry->card)[j]);
    } 
}

void MEMORY_EEPROM::init(void)
{
    defaultCalled = false;

    if(MEMORY_HEADER_LEN < sizeof(DeviceInfo))
    {
        Serial.println(F("Memory Header is larger than space allocated"));   
        Serial.print(MEMORY_HEADER_LEN);
        Serial.print(F(" space avaliable "));
        Serial.print(sizeof(DeviceInfo));
        Serial.println(F(" taken."));
    }
}

bool MEMORY_EEPROM::storeNetworkInfo(DeviceInfo *device)
{
    unsigned int i;
    for(i=0;i<sizeof(DeviceInfo);i++)
    {
       EEPROM.write(i, ((byte*) device)[i]);
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
       Serial.println(F("Using Default IP - 192.168.1.200"));
       device->ip = IPAddress(192,168,1,200);
    }

    // Gateway Default
    if (device->gateway == IPAddress(0xFFFFFFFF))
    {
       defaultOutput(true);
       Serial.println(F("Using Default Gateway - 192.168.1.1"));
       device->gateway = IPAddress(192,168,1,1);
    }
    
    // Subnet Default
    if (device->subnet == IPAddress(0xFFFFFFFF))
    {
       defaultOutput(true);
       Serial.println(F("Using Default Subnet IP - 255.255.255.0"));
       device->subnet = IPAddress(255,255,255,0);
    }
    
    // Server Default
    if (device->server == IPAddress(0xFFFFFFFF))
    {
       defaultOutput(true);
       Serial.println(F("Using Default Server - 192.168.1.2"));
       device->server = IPAddress(192,168,1,2);
    }
    
    // Mac Address Default
    if(device->mac[0] != 0x02 || device->mac[1] != 0x08 || device->mac[2] != 0xDC)
    {
       defaultOutput(true);
       Serial.println(F("Using Default Mac Address - 02:08:DC:EF:FE:ED"));
       // Static OUI (Organizationally Unique Identifier)
       device->mac[0] = 0x02;
       device->mac[1] = 0x08;
       device->mac[2] = 0xDC;
       // NIC (Network Interface Controller)
       device->mac[3] = 0xEF;
       device->mac[4] = 0xFE;
       device->mac[5] = 0xED;
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
       device->deviceName[0] = 'd';
       device->deviceName[1] = 'e';
       device->deviceName[2] = 'f';
       device->deviceName[3] = 'a';
       device->deviceName[4] = 'u';
       device->deviceName[5] = 'l';
       device->deviceName[6] = 't';
       device->deviceName[7] = '\0'; // End char
    }
    
    
    // Device Id Default
    if (device->id == 0xFFFF || ! valid)
    {
       defaultOutput(true);
       Serial.println(F("Using Default Id - 0"));
       device->id = 0;
    }
    
    defaultOutput(false);
    return true;
}

void MEMORY_EEPROM::defaultOutput(boolean isStart)
{
    if(isStart && !defaultCalled)
    {
       defaultCalled = true;
       Serial.println(F("-- No config found in memory, erasing EEPROM memory, and using defaults --"));
       
       MEMORY.erase();
       //MEMORY.storeNetworkInfo(&mySettings); 
       Serial.println("EEPROM erase done");
    }
    else if(!isStart && defaultCalled)
    {
       Serial.println(F("----------------------------------------------"));
    }
}

// Find if card is in memory, if so update the time, if not add it.
bool MEMORY_EEPROM::storeAccess(RFID_info *access)
{
    unsigned int i,j;
    RFID_info entry;
    
    for(i=MEMORY_HEADER_LEN;;i+=MEMORY_RFID_LENGTH)
    {
        for(j=0;j<MEMORY_RFID_LENGTH;j++)
        {
            ((byte*) &entry)[j] = EEPROM.read(i+j); 
        }
        
        if(entry.card != 0xFFFFFFFF)
        {
            if(entry.card == access->card)
            {
                // Write new data to the same location
                 _write_to_eeprom_at_fixed_offset(access, i);
                return true;
            }
        }
        else
        {
             // Write new data to this empty slot
             _write_to_eeprom_at_fixed_offset(access, i);
             
             // Write new empty slot after data, to be sure. 
             entry.card = 0xFFFFFFFF;
             i+=MEMORY_RFID_LENGTH;
             _write_to_eeprom_at_fixed_offset(&entry, i);
            return true;
        }
    }
    return false;
}



// Returns true if card is valid and hasn't expired
bool MEMORY_EEPROM::accessAllowed(unsigned long *rfid)
{
    unsigned int i,j;
    RFID_info entry;
    
    for(i=MEMORY_HEADER_LEN;;i+=MEMORY_RFID_LENGTH)
    {
        for(j=0;j<MEMORY_RFID_LENGTH;j++)
        {
            ((byte*) &entry)[j] = EEPROM.read(i+j); 
        }
        
        if(entry.card != 0xFFFFFFFF)
        {
            if(entry.card == *rfid)
            {
               return true;
            }
        }
        else
        {
            break;
        }
    }
    
    return false;
}

// Set the timestamp to zero
bool MEMORY_EEPROM::expireAccess()
{
    erase();
    return true;
    
     unsigned int i;
    
    for(i=MEMORY_HEADER_LEN;;i+=MEMORY_RFID_LENGTH)
    {
        EEPROM.write(i, 0xff);
    }
    return true; 
}

// Set the timestamp to zero
bool MEMORY_EEPROM::deleteUser(RFID_info *access)
{

    unsigned int i,j;
    RFID_info entry;

    int found_offset = 0;
  // locate user in eeprom: 
  for(i=MEMORY_HEADER_LEN;;i+=MEMORY_RFID_LENGTH)
    {
        for(j=0;j<MEMORY_RFID_LENGTH;j++)
        {
            ((byte*) &entry)[j] = EEPROM.read(i+j); 
        }
        
        if(entry.card != 0xFFFFFFFF)
        {
            if(entry.card == access->card)
            {
               found_offset = i;
               break; // no need to look further, we found it.
            }
        } else { 
          // last entry is always 0xFFFFFFFF, we're done looking. 
          break;
        }
    }
    
    


    for(i=MEMORY_HEADER_LEN;;i+=MEMORY_RFID_LENGTH)
    {
//        EEPROM.write(i, 0xff);
    }
    return true; 
}


// Print access list and timestamps
void MEMORY_EEPROM::printAccessList(void)
{   
    unsigned int i,j;
    RFID_info entry;
    
    for(i=MEMORY_HEADER_LEN;;i+=MEMORY_RFID_LENGTH)
    {
        for(j=0;j<MEMORY_RFID_LENGTH;j++)
        {
            ((byte*) &entry)[j] = EEPROM.read(i+j); 
        }
        
        if(entry.card != 0xFFFFFFFF)
        {
            Serial.println(entry.card);
        }
        else
        {
            break;
        }
    }
}

bool MEMORY_EEPROM::erase(void)
{
    for (int i = 0; i < 512; i++) {
      EEPROM.write(i, 0xff);
    }
    
    return true;   
}

MEMORY_EEPROM eepromMemory;
