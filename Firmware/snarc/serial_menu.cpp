/*
 *   serial_menu.c - Give the user a serial menu to program from
 *   Copyright (C) 2013 Luke Hovigton, David Bussenschutt. All right reserved.
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
#define SERIAL_TIMEOUT_COUNT 10

/******************************************************************************
 * Constructors
 ******************************************************************************/

/******************************************************************************
 * User API
 ******************************************************************************/

void SERIAL_MENU::init(int baud)
{
  Serial.begin(baud);
  Serial.println("DEVICE REBOOTED -- DEVICE REBOOTED -- DEVICE REBOOTED -- DEVICE REBOOTED -- DEVICE REBOOTED -- DEVICE REBOOTED --\n To enter program mode type '+++'\n");
  state = START;
}

// Check for +++, then enter program mode..
void SERIAL_MENU::check(void)
{
    if (Serial.available())
    {
        switch(state)
        {
            case START:
            case PLUS1:
                if (Serial.read() == '+')
                {
                    state++;
                }
                else
                {
                    state = START;
                }
                break;
            case PLUS2:
                state = START;
                if (Serial.read() == '+')
                {
                    display();
                }
                break;
        }
    }
}

extern unsigned long last_code; 

void SERIAL_MENU::display(void)
{
    char incomingByte = 0;
    DeviceInfo   mySettings;
    RFID_info    newCard;
    IPAddress    tempIP;
    
    MEMORY.getNetworkInfo(&mySettings);
    changesMade = false;
    
    LEDS.off(LEDS_ALL);
    clear_serial_buffer();
    Serial.println(F("Entered Programming Mode! "));
    prompt();
    
    while( incomingByte != -1 )
    {
        LEDS.toggle(LEDS_RED, 400); // RED FLASH FOR PROGRAMMING MODE
        
        if ( Serial.available() )
        {
            incomingByte = Serial.read();
            Serial.println((char)incomingByte);
            
            switch((char)incomingByte)
            {
              
                case 'y':  // both s and r are taken. :-) 
                  SERIAL_MENU::software_Reset(); 
                break;
                
                // Read current list from EEPROM cache
                case 'r':  // 'r'ead cards
                    Serial.println(F("Card list.."));
                    MEMORY.printAccessList();
                    break;
                                   
                // Write new code to MEMORY
                // the next key scanned will be saved
                //case 'k':
                case 'n': // add 'new' card:
                    //RFID.clear(); // clear buffers before we ask for data to be scanned.
                    Serial.println(F("Scan new card now..."));
                    delay(1000);
                    while(!RFID.available()) { delay(100); } 
                    delay(10); // for rest of RFID buffer to get here 
//                    // Wait for card to be read
                    last_code = 0;
                    int x;
//                    for (int y = 0 ; y < 10; y++ ) { 
                    x = RFID.read();   // puts read val into global 'unsigned long last_code'
//                    Serial.println(x); 
//                    if ( x == false ) x = ; 
//                    }

                    //last_code = 9999999; // HACK TEST
                    newCard.card = last_code; 
                    Serial.println((uint32_t)newCard.card); 

                    MEMORY.dumphead();
                    
                    if(MEMORY.storeAccess(newCard))
                    {
                        MEMORY.dumphead();
                        Serial.print(F("-- "));
                        Serial.print((unsigned long)newCard.card);
                        Serial.println(F(" STORED --"));
                    }
                    else
                    {
                        Serial.println(F("ERROR ADDING CARD!"));  
                    }
                    break;
                
                // Erase/Initialise different bits of MEMORY
                case 'i':
                    MEMORY.erase_network_settings();
                    Serial.println(F("Configuration MEMORY  - Erase complete.. ( rfid tags untouched) "));
                    break;

                // TEST ( and wipe )  all MEMORY - dangerous. 
                case 'j':

                    for ( int w = 0 ; w < 128 ; w++ ) { 
                      MEMORY.test_and_wipe_eeprom();
                    }
                    Serial.println(F("Memory Test ( and wipe )  complete.."));
                    break;

                // Erase/Initialise all MEMORY
                case 'k':
                    MEMORY.erase_rfid_tags();
                    Serial.println(F("TAG Memory  - Erase complete. ( configuration memory untouched) "));
                    break;
                    

//                //  w means "write" hard-coded LIST ( from to EEPROM cache - undocumented command for initial population of eeprom
//                case 'w':    
//                    Serial.println(F("please wait, writing new codes list...."));
//
//                    //write_codes_to_eeprom(); // TODO
//                    Serial.print(F("address:"));
//                    //Serial.println(last_address);
//                    prompt();
//                  break;
                
                // Expire a cards, typed or scanned
//                case 'z':
//                    
//                    if(MEMORY.expireAccess())
//                    {
//                        Serial.println(F("-- ALL CARDS REMOVED --")); 
//                    }
//                    break;
//                
                // Ask server for card update
                case 'u':
                    Serial.println(F("Updating card list"));
                    //Serial.println(send_to_server("1234567890", 0));
                    break;
                
                // --------------  Server update functions -------------- //
                // ---  These need to be saved after updating them ------ //
                
                // Set Device Name
                case 'd':
                    listen_for_device_name(mySettings.deviceName);
                    break;
                
                // Set Device Id
                case 't':
                    listen_for_device_id(&mySettings.id);
                    break;
                
                // Set Mac Address
                case 'm':
                    Serial.println(F("disabled in implementation sorry ( buggy ) , use hardcode MACs only."));
                    // set MAC address
                    listen_for_new_mac_address();
                    // HACK: we let the user change the first three octets, but then force them back to their hardcode value, so all our devices have the same manufacturer prefix 
                    mySettings.mac[0] = 0x02;
                    mySettings.mac[1] = 0x08;
                    mySettings.mac[2] = 0xDC;
                    // NIC (Network Interface Controller)
                    // TO MANUALLY FORCE A SPECIFIC MAC AT COMPILE TIME, UNCOMMENT THIS: 
//                    mySettings.mac[3] = 0xEF;
//                    mySettings.mac[4] = 0xFE;
//                    mySettings.mac[5] = 0xEF;
                    changesMade = true;
                    break;
                
                // Set Ip Address
                case 'b':
                    tempIP = mySettings.ip;
                    listen_for_ipaddress(&tempIP);
                    mySettings.ip = tempIP;
                    break;
                
                // Set Gateway Address
                case 'g':
                    tempIP = mySettings.gateway;
                    listen_for_ipaddress(&tempIP);
                    mySettings.gateway = tempIP;
                    break;

//                // Set Subnet Address
//                case 't':
//                    tempIP = mySettings.subnet;
//                    listen_for_ipaddress(&tempIP);
//                    mySettings.subnet = tempIP;
//                    break;
                    
                // Set Server Address
                case 'a':
                    tempIP = mySettings.server;
                    listen_for_ipaddress(&tempIP);
                    mySettings.server = tempIP;
                    break;
                    
                // Save Changed data
                case 's':
                    if(changesMade)
                    {
                       MEMORY.storeNetworkInfo(&mySettings); 
                       changesMade = false;
                       Serial.println(F("Changes Saved.."));
                       Serial.println(F("Reset device for settings to take effect"));
                    }
                    else
                    {
                       Serial.println(F("No changes made.."));
                    }
                    break;

                case 'p':
                    print_node_config(&mySettings);
                    break;
                    
                case 'e':
                    ETHERNET.print_settings();
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
                    
                // nothing
                case 'h':
                default:
                    Serial.print(F("sorry, I don't understand what this means:"));  Serial.print((char)incomingByte);  Serial.print(" "); Serial.println(incomingByte,HEX);
                    while ( Serial.available() > 0 ) Serial.read(); // flush serial input 
                    prompt();
                    break;
            } //switch/case
            if(incomingByte != -1)
            {
                Serial.println(F("Command Done.."));
            }
            clear_serial_buffer();
        } // if
    } //while
    
    LEDS.off(LEDS_ALL);
    Serial.println("Exited Program Mode");
}

extern int freeMemory();
extern int freeRam(void);


void SERIAL_MENU::prompt(void)
{
    Serial.println(F("\nPROGRAM MODE:"));
    //Serial.println(F("For Command List goto: https://github.com/HSBNE/snarc_code"));
    Serial.println(F("r - print card list"));
    Serial.println(F("n - program new key to MEMORY"));
    //Serial.println(F("z - expire a single card from MEMORY"));
    Serial.println(F("u - Ask server to give us a card update"));
   // Serial.println(F("w - write card list to MEMORY"));
    Serial.println(F("i - erase_network_settings from in EEPROM. \n"));
    Serial.println(F("j - write-then-read test EEPROM (dangerous, does not clear FF to eeprom after. u need to use i and k after this to reset eeprom) \n"));
    Serial.println(F("k - erase_rfid_tags from in EEPROM \n"));
    
    Serial.println(F("-- Options below need to be saved after change --"));
    Serial.println(F("d - set device name"));
    Serial.println(F("t - set device identification"));
    Serial.println(F("m - set MAC address"));
    Serial.println(F("b - set IP address"));
    Serial.println(F("g - set gateway address"));
    //Serial.println(F("n - set subnet address"));
    //Serial.println(F("e - set DNS address")); // -- Not supported yet -- //
    Serial.println(F("a - set server address"));
    Serial.println(F("s - save changes"));
    Serial.println(F("p - print node config"));
    Serial.println(F("e - print loaded ethernet settings\n"));
    
    Serial.println(F("x - exit programming mode"));
    Serial.println(F("y - soft reboot"));

    MEMORY.dumphead();

    Serial.print(F("freeMemory()="));
    Serial.println(freeMemory());
    Serial.print(F("freeRAM()="));
    Serial.print(freeRam());

}

void SERIAL_MENU::print_node_config(DeviceInfo *settings)
{
    Serial.println(F("---------------- Local config ----------------"));
    
    Serial.print(F("Device Name: "));
    Serial.println((String) settings->deviceName);

    Serial.print(F("Device ID: "));
    Serial.println((int)settings->id);
    
    Serial.print(F("IPAddress:"));
    Serial.println((IPAddress) settings->ip);
    
    Serial.print(F("Gateway:  "));
    Serial.println((IPAddress) settings->gateway);
    
    Serial.print(F("Subnet:   "));
    Serial.println((IPAddress) settings->subnet);

    Serial.print(F("Server:   "));
    Serial.println((IPAddress) settings->server);

    // We know the first Half as it's Static, lets save some code space and just print it :)
    Serial.print(F("Mac:      02:08:DC:"));
    Serial.print(settings->mac[3],16);
    Serial.print(F(":"));
    Serial.print(settings->mac[4],16);
    Serial.print(F(":"));
    Serial.println(settings->mac[5],16);
    
    Serial.println(F("----------------------------------------------"));
    
}

void(* resetFunc) (void) = 0;//declare reset function at address 0
void SERIAL_MENU::software_Reset() // Restarts program from beginning but does not reset the peripherals and registers
{
// asm volatile ("  jmp 0");  
Serial.println("Doing software reset now.....");  Serial.flush(); delay(100); 
resetFunc();
} 

void SERIAL_MENU::listen_for_new_mac_address(){ // pass in 1,2,3 or 4 to this when called.


   Serial.print(F("Current MAC is: "));
    for( int oct = 0 ; oct < 6 ; oct ++ ) { 
      if ( mySettings.mac[oct] < 15 ) { Serial.print(F("0")); } // leading zero on hex digits below 'F'
      Serial.print(mySettings.mac[oct],16); 
      if ( oct < 5 ) { Serial.print(F(":")); }
    }
    Serial.print(F("  "));

    Serial.println(F("Enter new MAC eg 02:08:DC:01:02:D3. At HSBNE, first three octets should stay as 02:08:DC:"));
    
    boolean keepReading = true;
    int     serial_recieve_index = 0;

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
            globalBuffer[serial_recieve_index++] = Serial.read();
            if (serial_recieve_index >= 18) // there is a max of 18 chars - ff:ff:ff:ff
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
    else {
        Serial.println(F("Scanning input for Changes...."));

      // Proccess the ip address and make it a number 
       int part[6];

       serial_recieve_index = sscanf(globalBuffer, "%x:%x:%x:%x:%x:%x", part, part+1, part+2, part+3,part+4,part+5); // %x means parse for hexadecimal number/s 
       Serial.println(serial_recieve_index);
       if(serial_recieve_index == 6)
       {

          mySettings.mac[0] = part[0];
          mySettings.mac[1] = part[1];
          mySettings.mac[2] = part[2];
          mySettings.mac[3] = part[3];
          mySettings.mac[4] = part[4];
          mySettings.mac[5] = part[5];
          
          Serial.print(F("NEW MAC is: "));
          for( int oct = 0 ; oct < 6 ; oct ++ ) { 
            Serial.print(mySettings.mac[oct],16); 
            if ( oct < 5 ) { Serial.print(F(":")); }
          }
          Serial.print(F("  "));

          changesMade = true;
       }

    }
}

void SERIAL_MENU::listen_for_ipaddress(IPAddress *change)
{
    Serial.println(F("Current IP is: "));
    Serial.println(*change);
    Serial.println(F("Enter new IP eg 192.168.1.1"));
    
    boolean keepReading = true;
    int     serial_recieve_index = 0;
    
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
            globalBuffer[serial_recieve_index++] = Serial.read();
            if (serial_recieve_index >= 15) // there is a max of 15 chars - 255.255.255.255
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
       // Proccess the ip address and make it a number 
       int part[4];

       serial_recieve_index = sscanf(globalBuffer, "%d.%d.%d.%d", part, part+1, part+2, part+3);
       if(serial_recieve_index == 4)
       {
          *change = IPAddress(part[0], part[1], part[2], part[3]);
          Serial.print(F("IP set to: "));
          Serial.println(*change);
          changesMade = true;
       }
    }
}

void SERIAL_MENU::listen_for_device_name(char *deviceName)
{
    boolean keepReading = true;
    int     serial_recieve_index = 0, i;
            
    Serial.print(F("Current device name is: "));
    Serial.println(deviceName);
    Serial.print(F("Enter new device name (max "));
    Serial.print(MEMORY_DEVICE_NAME_MAX_LENGTH-1); // We need a termination char
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
            deviceName[serial_recieve_index++] = Serial.read();
            if (serial_recieve_index >= MEMORY_DEVICE_NAME_MAX_LENGTH-1)
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
            deviceName[i] = '\0';
        }
        
        changesMade = true;
        // Echo & save new name
        Serial.print(F("Device name set to: "));
        Serial.println(deviceName);
    }
}

void SERIAL_MENU::listen_for_device_id(unsigned long int *deviceId)
{
    boolean keepReading = true;
    int     serial_recieve_index = 0;
    char    deviceChar[6];
            
    Serial.print(F("Current device id is: "));
    Serial.println(*deviceId);
    Serial.print(F("Enter new device Id"));
    
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
            deviceChar[serial_recieve_index++] = Serial.read();
            if (serial_recieve_index >= 5)
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
        deviceChar[serial_recieve_index] = '\0';
        sscanf(deviceChar, "%lu", deviceId);
        changesMade = true;
        
        Serial.print(F("Device ID set to: "));
        Serial.println(*deviceId);
    }
}

// There is lots of delay here cos we want to make sure nothing is left in the buffer..
// But still be responsive to the user.
void SERIAL_MENU::clear_serial_buffer(void)
{
    int timeout = 0;
    while(!Serial.available() && (timeout++ < SERIAL_TIMEOUT_COUNT)){delay(1);}
    while (Serial.available())
    {
        Serial.read();
        timeout = 0;
        while(!Serial.available() && (timeout++ < SERIAL_TIMEOUT_COUNT)){delay(1);}
    }
}

SERIAL_MENU SNARC_SERIAL_MENU;
