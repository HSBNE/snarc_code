/*
 *   serial_menu.c - Give the user a serial menu to program from
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

#include "serial_menu.h"

/******************************************************************************
 * Definitions
 ******************************************************************************/

//#define DEBUG

/******************************************************************************
 * Constructors
 ******************************************************************************/

/******************************************************************************
 * User API
 ******************************************************************************/

void SERIAL_MENU::display(void)
{
    clear_serial_buffer();
    Serial.println(F("Press a few keys, then ENTER *NOW* to start programming mode ( 3...2...1... ) "));

    delay(3000);

    // three ENTER keys is >2, AND WE ARE IN PROGRAMMING MODE !  
    int incomingByte = 0;

#ifdef DEBUG
    if (true) // DEBUG: Always enter programming mode
#else
    if (Serial.available() > 2)
#endif
    {
        clear_serial_buffer();
        Serial.println(F("Entered Programming Mode! "));
        prompt();
        
        while( incomingByte != -1 )
        {
            LEDS.blink(LEDS_RED); // RED FLASH FOR PROGRAMMING MODE
            
            if ( Serial.available() )
            {
                incomingByte = Serial.read();
                Serial.println((char)incomingByte);
                
                switch((char)incomingByte)
                {
                    // Test Server Connection
                    case 's':
                        Serial.println(F("testing send_to_server() function"));
                        //Serial.println(send_to_server("1234567890", 0));
                        prompt();
                        break;
#ifdef CLIENT
                    // Set Mac Address
                    case 'm':
                        Serial.println(F("disabled in implementation sorry ( buggy ) , use hardcode MACs only."));
                        // TODO: Implement IP address change
                        // set MAC address
                        //listen_for_new_mac_address();
                        prompt();
                        break;
                    
                    // Set Ip Address
                    case 'a':
                        Serial.println(F("not yet implemeted, sorry."));
                        // TODO: Implement IP address change
                        prompt();
                        break;
#endif
                    // Set Device Name
                    case 'd':
                        listen_for_device_name();
                        prompt();
                        break;
                        
// w means "write" initial LIST to EEPROM cache - undocumented command for initial population of eeprom only during transition.
                    case 'w':
                        // generally yuou should do the 'i' before 'w'
                        // Serial.println(F("please wait, erase in progress...."));
                        // init_eeprom();
//                        Serial.println(F("please wait, writing codes...."));
//                        write_codes_to_memory();
//                        Serial.print(F("address:"));
//                        Serial.println(last_address);
                        prompt();
                        break;
                    
                    // Erase Data
                    case 'i':
                        Serial.println(F("please wait, erase in progress...."));
                        MEMORY.erase_access_list(); //wipe all codes
                        prompt();
                        break;
                    
                    // r mean read current list from EEPROM cache
                    case 'r':
                        MEMORY.print_access_list();
                        prompt();
                        break;
                    
                    // z means delete a single key from EEPROM without deleting all of them.
                    case 'z':
//                        MEMORY.delete_code();
//                        Serial.print(F("address:"));
//                        Serial.println(last_address);
                        prompt();
                        break;

                    // x mean exit programming mode, and resume normal behaviour
                    case 'x':
                        incomingByte = -1; // exit this mode
                        break;
                    
                    // ignore whitespace
                    case '\r':
                    case '\n':
                    case ' ':
                        break;
                        
                    // n means write new code to EEPROM
                    // ( the next key scanned
                    case 'n':
//                        listen_for_codes();
                        // result is in last_door and last_code
                        DOOR.unlockDoor(2000); // clunk the door as user feedback 2 seconds
                        // see if the key already has access
//                        int access = matchRfid(last_code) & 0xF;
//                        Serial.print(F("EEPROM access level:"));
//                        Serial.println(access);
                        
                        // IF THIS DOOR/READER is not already in the permitted access list from the EEPROM/SERVER allow it!
                        // this converts the bit number, to its binary equivalent, and checks if that bit is set in "access"
                        // by using a binary "and" command.
//                        Serial.print(F("THISSNARC:"));
//                        Serial.print(THISSNARC);
//                        Serial.print(F(" bits:"));
//                        Serial.print((  1 << ( THISSNARC - 1 ) ));
//                        Serial.print(F(" more bits:"));
//                        Serial.println(access & (  1 << ( THISSNARC - 1 ) ));
//                        
//                        if ( (access & (  1 << ( THISSNARC - 1 ) )) ==  0 )
//                        {   // ie no access yet for this door, yes all these brackets are needed
//                            // append this ocde to the EEPROM, with current doors permissions+ new one
//                            write_next_code_to_eeprom(last_code, access | (  1 << ( THISSNARC - 1 ) ));
//                        }
//                        else
//                        {
//                            Serial.println(F("Card already has access, no change made"));  
//                        }
//                        prompt();
                        break;
                        
                        // nothing
                        default:
                            prompt();
                            break;
                } //switch/case
                clear_serial_buffer();
            } // if
        } //while
    }
}


void SERIAL_MENU::prompt(void)
{
    Serial.println();
    Serial.println(F("PROGRAM MODE:"));
    Serial.println(F("r - read eeprom list"));
    Serial.println(F("n - program new key to EEPROM"));
    Serial.println(F("s - test server interface ( sends fake tag 1234567890 to server ) "));
    Serial.println(F("d - set device name"));
    Serial.println(F("i - wipe and initialise EEPROM (dangerous!) "));
    Serial.println(F("w - write hard-coded tags to EEPROM (dangerous!)"));
    Serial.println(F("z - delete a single card from EEPROM"));
#ifdef CLIENT
    Serial.println(F("m - set/reset MAC address"));
    Serial.println(F("a - set device IP address"));
#endif
    Serial.println();
    Serial.println(F("x - exit programming mode"));
}

void SERIAL_MENU::clear_serial_buffer(void)
{
    while (Serial.available())
    {
        Serial.read();
    }
}

void SERIAL_MENU::listen_for_device_name(void)
{
    bool keepReading = true;
    char serial_recieve_data[MEMORY_DEVICE_NAME_MAX_LENGTH];
    int  serial_recieve_index = 0;
    int  i;
            
    Serial.print(F("Current device name is: "));
    if(MEMORY.get_device_name(serial_recieve_data))
    {
        Serial.print(serial_recieve_data);
        Serial.println();
    }
    else
    {
        Serial.println(F("Failed to get device name"));
    }
   
    Serial.print(F("Enter new device name (max "));
    Serial.print(MEMORY_DEVICE_NAME_MAX_LENGTH);
    Serial.println(F(" characters):"));
    
    clear_serial_buffer();
    
    while (keepReading)
    {
        while (Serial.available())
        {
            if (Serial.peek() == 13 || Serial.peek() == 10)
            {
                // new line. End entry
                keepReading = false;
                break;
            }
            serial_recieve_data[serial_recieve_index++] = Serial.read();
            if (serial_recieve_index >= MEMORY_DEVICE_NAME_MAX_LENGTH)
            {
                // max length. End entry
                keepReading = false;
                break;
            }
        }
    }
    clear_serial_buffer();
    
    if (serial_recieve_index == 0)
    {
        // Empty, do not save.
        Serial.println(F("No input detected. No changes made."));
    }
    else
    {
        // Fill rest of device name array with null chars
        for (i = serial_recieve_index; i <= MEMORY_DEVICE_NAME_MAX_LENGTH; i++)
        {
            serial_recieve_data[i] = '\0';
        }
        
        MEMORY.set_device_name(serial_recieve_data);
        // Echo & save new name
        Serial.print(F("Device name set to: "));
        Serial.print(serial_recieve_data);
        Serial.println();
    }
}

SERIAL_MENU SNARC_SERIAL_MENU;
