/*
 *   ethernet_http.cpp - Ethernet libary using calls to a http server for information
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

#include "ethernet_http.h"

/******************************************************************************
 * Definitions
 ******************************************************************************/

/******************************************************************************
 * Constructors
 ******************************************************************************/

/******************************************************************************
 * User API
 ******************************************************************************/

EthernetServer localserver(80);
void ETHERNET_HTTP::init(byte *mac, IPAddress ip, IPAddress gateway, IPAddress subnet, IPAddress server)
{
#ifdef ETHERNET_RESET_PIN
    pinMode(ETHERNET_RESET_PIN, OUTPUT);
    digitalWrite(ETHERNET_RESET_PIN, HIGH);
#endif
    delay(1000);   // delay boot by precautionary 1sec 
    Ethernet.begin(mac,ip,gateway,subnet); //, ETHERNET_CS);    
    serverIP = server;
    localserver.begin();
}



void ETHERNET_HTTP::print_settings(void)
{
    __attribute__ ((unused)) byte macAdd[6];
    
    Serial.println(F("--------- Ethernet Setting from chip ---------"));
    
    Serial.print(F("IPAddress:"));
    Serial.println(Ethernet.localIP());
    
    Serial.print(F("Gateway:  "));
    Serial.println(Ethernet.gatewayIP());
    
    Serial.print(F("Subnet:   "));
    Serial.println(Ethernet.subnetMask());

    Serial.print(F("DNS:      "));
    Serial.println(Ethernet.dnsServerIP());

//    Ethernet.macAddress(macAdd);
//    Serial.print(F("Mac:      "));
//    Serial.print(macAdd[0],16);
//    Serial.print(F(":"));
//    Serial.print(macAdd[1],16);
//    Serial.print(F(":"));
//    Serial.print(macAdd[2],16);
//    Serial.print(F(":"));
//    Serial.print(macAdd[3],16);
//    Serial.print(F(":"));
//    Serial.print(macAdd[4],16);
//    Serial.print(F(":"));
//    Serial.println(macAdd[5],16);

    Serial.print(F("Server:   "));
    Serial.println(serverIP);

    Serial.println(F("----------------------------------------------"));   
}

int ETHERNET_HTTP::check_tag(unsigned long *tag, unsigned long int *door, char *name)
{
   EthernetClient client;
   int client_recieve_pointer = 0, x = 0;

   if (client.connect(serverIP,80))
   {
      Serial.println(F("http client connected"));
      client.print("GET /logger.php?secret=asecret&q=");
      client.print(*tag);
      client.print("&d=");
      client.print(*door);
      client.print("&n=");
      client.println(*name);
      client.println();
      
      // note the time that the connection was made:
      ethernetWiznetChecker.last_connection_time();
    }
    else
    {
    	Serial.println(F("http connection failed"));
        client.stop();
    	return -1; // error code to say server offline, which is different to "0" , which means deny access.
    }
    
    // delay some arbitrary amount for the server to respond to the client. say, 1 sec. ?
    delay(2000);
    
    for (x=0;x<=GLOBAL_BUFFER_LEN;x++)  //client_recieve_data is only 32 bytes long.
    {
        if (client.available())
        {
            char c = client.read();
            globalBuffer[client_recieve_pointer++] = c;
        }

        // if the server's disconnected, stop reading info
        if (!client.connected())
        {
       	    break;
        }
    }
    
    // We have all the info we need, so disconnect..
    client.stop();
    Serial.println(F("http client finished"));
    
    if ( x >= GLOBAL_BUFFER_LEN )
    {
        Serial.println(F("too much HTTP data, error! ( do you have an logger.php on the server? ) "));
        Serial.print(F("http data:"));
        globalBuffer[x] = '\0';
        Serial.println(globalBuffer);
        return -1;
    }

    client_recieve_pointer = 0;
    // if the server's disconnected, stop the client:
    Serial.print(F("http data:"));
    // recieved data is now in the string: client_recieve_data
    globalBuffer[x] = '\0';
    Serial.println(globalBuffer);
    
    // we expect the permissions string to look like 'access:1' ( for permit ), or 'access:0' (for deny )    
    for (x=0;x<=GLOBAL_BUFFER_LEN;x++)
    {
        if( globalBuffer[x] == ':')
        {
           x++;
           break; 
        }
    }
    
    // globalBuffer is a pointer, we are changing where this points to by adding x
    int ci = atoi(globalBuffer+x); // as an int!  if this fails, it returns zero, which means no-access, so that's OK.
    Serial.print(F("perms from server:"));
    Serial.println(ci);
    
    // basic bound check,  return -1 on error .
    if ( ci < 0 || ci > 255 ) {
       return -1;   
    }
    return ci;
}


void ETHERNET_HTTP::listen(void)
{
  // listen for incoming clients
  EthernetClient incomingclient = localserver.available();
  int x = 0;
  //int get_post = 0; // not_found = 0, get = 1, post = 2
  
  if (incomingclient)
  {
      // an http request ends with a blank line
      boolean currentLineIsBlank = true;
      while (incomingclient.connected())
      {
          if (incomingclient.available())
          {
              char c = incomingclient.read();
              //Serial.print(c); // debug to show client data on Serial console.
              // if you've gotten to the end of the line (received a newline
              // character) and the line is blank, the http request has ended,
              // so you can send a reply
              
              //read char by char HTTP request into buffer, toss rest away!
              if (x++ < GLOBAL_BUFFER_LEN)
              {
                  //store characters to string
                  globalBuffer[x] = c;
              }
              if (c == '\n' && currentLineIsBlank)
              {
                globalBuffer[x] = 0;
                
                  Serial.println(globalBuffer);
                  
                  
                  if ( globalBuffer[0] == ';' ) {   
                      
                    MEMORY.erase_rfid_tags();  // clear all cards in eeprom , but leave network settings and config
                    
                    
                    RFID_info a; 
                    a.card = atol(globalBuffer+1);
                   // globalBuffer--;
                    // everything between the tag and the \n is nominally a RFID tag to write into te eeprom.
                    MEMORY.storeAccess(a); 
                 // } 
                    
                    
                  } 
       
                  // send a standard http response header
                  incomingclient.println(F("HTTP/1.1 200 OK"));
                  incomingclient.println(F("Content-Type: text/html"));
                  incomingclient.println();
                  
                  incomingclient.println(F("<HTML>"));
                  incomingclient.println(F("<BODY>"));

                  // control arduino pin
                  for(x=0; x< GLOBAL_BUFFER_LEN; x++)
                  {
                    if( globalBuffer[x] == 'o' || globalBuffer[x] == 'c' )
                    {
                       break; 
                    }
                  }
                  if(x > 4 && globalBuffer[x] == 'c' && globalBuffer[x+2] == 'o')//checks for close
                  {
                      DOOR.lock();
                      Serial.println(F("Door closed."));
                  }
                  else if(x > 4 && globalBuffer[x] == 'c' && globalBuffer[x+2] == 'e')//checks for clear
                  {
                      Serial.println(F("Mem cleared."));
                      MEMORY.erase_rfid_tags();  // clear all cards in eeprom 
                  }
                  else if(x > 4 && globalBuffer[x] == 'o' && globalBuffer[x+1] == 'p')//checks for open
                  {
                      DOOR.open();
                      Serial.println(F("Door open."));
                  }

                  
                  incomingclient.println(F("</BODY>"));
                  incomingclient.println(F("</HTML>"));
                  
                  break;
                }
                    
                if (c == '\n')
                {
                    // you're starting a new line
                    currentLineIsBlank = true;
                }
                else if (c != '\r')
                {
                    // you've gotten a character on the current line
                    currentLineIsBlank = false;
                }
            }
        }
        // give the web browser time to receive the data
        delay(1);
        // close the connection:
        incomingclient.stop();
    }
} // END WEB SERVER/LISTNER CODE:


void ETHERNET_WIZNET_CHECKER::init(void)
{
  pollingInterval = 60;
  lastConnectionTime = 0;
}

    // If we've not had a successful http request in the last 60 seconds, then try to make one
    // if we've still not had a sucessful http request in hte last 60 seconds, then reset the wiznet module, wait 60 secs and try again.

void ETHERNET_WIZNET_CHECKER::listen(void)
{  
      if ((unsigned long)(millis() - lastConnectionTime) > (unsigned long)(pollingInterval*1000) ) {
         Serial.println(F("network poll checking now.... "));

         // do the poll/check with a dummy key, for now.
       int serveraccess = -1; //
       // serveraccess = send_to_server2("1234567890", 0); //log successes/failures/
       unsigned long test_code = 1234567890;
       serveraccess = ETHERNET.check_tag(&test_code, &mySettings.id, mySettings.deviceName);
       
        //etc, and return the permissions the server has.
        if ( serveraccess == -1 ) {
          Serial.println(F("network appears offline, forcing wiznet reset "));
          lastConnectionTime = millis(); // to make this code block not run again for at least 60 seconds, which is enough time ot bring wiznet online.
          wiznet_reset();
          delay(200);
          // re-init wiznet stackand whatnot.
              ETHERNET.init(mySettings.mac, mySettings.ip, mySettings.gateway, mySettings.subnet, mySettings.server);

         // Ethernet.begin(mac, ip, gateway, subnet);
         // localserver.begin();
          
        } else {
          Serial.println(F("network check appears OK. "));
        }
      }
  
  
}

void ETHERNET_WIZNET_CHECKER::wiznet_reset(void)
{ 
#ifdef ETHERNET_RESET_PIN
  //WIZRESET - ~30-48 secs after this, it pings - this is probably overkill, but I'm not sure if it's leading or trailing edge trggered, so I do both
  digitalWrite(ETHERNET_RESET_PIN, LOW);
  delay(50);
  digitalWrite(ETHERNET_RESET_PIN, HIGH);
  delay(200);  
#endif
}

void ETHERNET_WIZNET_CHECKER::last_connection_time() { 
   lastConnectionTime = millis();
} 

ETHERNET_WIZNET_CHECKER ethernetWiznetChecker;

ETHERNET_HTTP ethernetHttp;
