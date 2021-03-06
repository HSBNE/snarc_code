/*
 *   snarc_plus_leds.cpp - Controlls the onboard leds on the snarc+ board 
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

#include "leds_generic.h"

/******************************************************************************
 * Definitions
 ******************************************************************************/
unsigned int  pins[] = LED_PINS;

/******************************************************************************
 * Constructors
 ******************************************************************************/

/******************************************************************************
 * User API
 ******************************************************************************/

void LEDS_GENERIC::init()
{
  unsigned int i;
  for(i=0; i<LED_DEFINED; i++)
  {
    pinMode(pins[i], OUTPUT);
#ifndef INVERT_LEDS
    digitalWrite(pins[i],    LOW);
#else
    digitalWrite(pins[i],    HIGH);
#endif
  }

#ifndef INVERT_LEDS
  leds = 0;
#else
  leds = LEDS_ALL;
#endif
}

void LEDS_GENERIC::show_leds(unsigned int changed, int intensity)
{
  unsigned int i;
  for(i=0; i<LED_DEFINED; i++)
  {
    /* has this led change? */
    if(changed & 1<<i)
    {
      if(leds & 1<<i)
      {
        analogWrite(pins[i],  intensity);
      }
      else
      {
        digitalWrite(pins[i], LOW);
      }
    }
  }
}

#ifndef INVERT_LEDS
void LEDS_GENERIC::on(unsigned int ledv)
#else
void LEDS_GENERIC::off(unsigned int ledv)
#endif
{
  unsigned int changed;
  changed = (~leds) & ledv;
  leds |= ledv;
  show_leds(changed, 0xff);
}

#ifndef INVERT_LEDS
void LEDS_GENERIC::off(unsigned int ledv)
#else
void LEDS_GENERIC::on(unsigned int ledv)
#endif
{
  unsigned int changed;
  changed = leds & ledv;
  leds &= ~ledv;
  show_leds(changed, 0x00);
}

void LEDS_GENERIC::toggle(unsigned int ledv)
{
  unsigned int changed  = ledv & leds;
  
  leds |= (~changed & ledv);
  leds &= ~changed;
  
  show_leds(ledv, 0xff);
}

void LEDS_GENERIC::toggle(unsigned int ledv, unsigned int miliseconds)
{
  if (millis() > ledToggle + miliseconds)
  {
    ledToggle = millis();
    toggle(ledv);
  }
}

void LEDS_GENERIC::blink(unsigned int ledv)
{
  toggle(ledv);
  delay(400);
  toggle(ledv);
}

void LEDS_GENERIC::pwm(unsigned int ledv, int value)
{  
  if (value > 0)
  {
    leds |= ledv;
  }
  else
  {
    leds &= ~ledv;
  }

  if (value < 0xff)
  {
    show_leds(ledv, value);
  }
  else
  {
    show_leds(ledv, 0xff);
  }
}

LEDS_GENERIC generic_leds;
