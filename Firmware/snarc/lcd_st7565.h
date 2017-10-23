#ifdef USE_LCD
/*
 *   lcd_st7565.cpp - Graphical display
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
 
#ifndef __LCD_ST7565_H__
#define __LCD_ST7565_H__

#include "config.h"
#include "Arduino.h"

class LCD_ST7565
{
    public:
        void          init(void);
        void          start(void);
        void          updateCounter(uint32_t time);
    private:
        void          formatFullTime(uint32_t time, uint8_t line, uint8_t offset);
        void          formatHourTime(uint32_t time, uint8_t line, uint8_t offset);
        void          formatHalfTime(uint32_t time, uint8_t line, uint8_t offset);
        void          int2str(char* buffer, register int i);
};

extern LCD_ST7565 LCD;

#endif /* __LCD_ST7565_H__ */

#endif
