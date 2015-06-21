/*
 *   serial_menu.h - Give the user a serial menu to program from
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
 
#ifndef __SERIAL_MENU_H__
#define __SERIAL_MENU_H__

#include "config.h"
#include "Arduino.h"
#include <IPAddress.h>

class SERIAL_MENU
{
    enum Serial_State { START, PLUS1, PLUS2 }; 
    int      state;
    boolean  changesMade;
    
    public:
        void init(int baud);
        void check(void);
        void display(void);
    private:
        void prompt(void);
        void print_node_config(DeviceInfo *settings);
        void write_codes_to_memory(void);
        void clear_serial_buffer(void);
        void listen_for_device_name(char *deviceName);
        void listen_for_device_id(unsigned long int *deviceId);
        void listen_for_ipaddress(IPAddress *change);
};

extern SERIAL_MENU SNARC_SERIAL_MENU;

#endif /* __INTERRUPT_HANDLE_H__ */
