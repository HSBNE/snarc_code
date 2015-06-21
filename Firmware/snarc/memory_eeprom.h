/*
 *   door.h - actuate the door (Open/Close)
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
 
#ifndef __MEMORY_EEPROM_H__
#define __MEMORY_EEPROM_H__

#include "config.h"
#include "Arduino.h"
#include <IPAddress.h>
//#include <Time.h>
#include <EEPROM.h>

#ifndef MEMORY_DEVICE_NAME_MAX_LENGTH
#define MEMORY_DEVICE_NAME_MAX_LENGTH 12 // Maximum length of the device name
#endif

#define MEMORY_RFID_LENGTH  sizeof(RFID_info)
#define MEMORY_HEADER_LEN   50 // Space allocated for the config header

class MEMORY_EEPROM
{
    boolean defaultCalled;
  
    public:
        void init(void);
        bool storeNetworkInfo(DeviceInfo *device);
        bool getNetworkInfo(DeviceInfo *device);
        
        bool storeAccess(RFID_info *access); 
        bool accessAllowed(unsigned long *rfid);
        bool expireAccess();
        bool deleteUser(RFID_info *access);
        
        void printAccessList(void);
        bool erase(void);
        bool _write_to_eeprom_at_fixed_offset(RFID_info *entry, int offset);
        
    private:
        void defaultOutput(boolean isStart);

};

extern MEMORY_EEPROM eepromMemory;

#endif /* __MEMORY_EEPROM_H__ */
