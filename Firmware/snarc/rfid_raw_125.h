
/*
 *   rfid_RAWstudio_125.h - 125Khz RFID Card reader by RAWstuidio (Electronic brick version) though this may work with others
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
 
#ifndef __RFID_125_RAW_H__
#define __RFID_125_RAW_H__

#include "config.h"
#include "Arduino.h"

#include <SoftwareSerial.h>

#ifndef RFID_RX_PIN
#define RFID_RX_PIN D15
#endif

#ifndef RFID_TX_PIN
#define RFID_TX_PIN D8
#endif

#ifndef RFID_BAUD_RATE
#define RFID_BAUD_RATE 9600
#endif

class RFID_RAW_125
{
    public:
        void init(void);
        
        // Try read a RFID Tag, last_code will be updated if tag found
        // Returns True on successful tag read, False otherwise
        boolean read(unsigned long *last_code);
    private:
        //long hex2dec(String hexCode);
        void clear(void);
};

extern RFID_RAW_125 RFIDRAW125;

#endif /* __RFID_125_RAW_H__ */
