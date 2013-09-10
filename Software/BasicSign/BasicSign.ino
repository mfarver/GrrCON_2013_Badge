/* Example scrolling signboard app for GrrCON 2013 badges */
/* Author: Mark Farver                                    */
/* Date:   26 Aug 2013                                    */
/*
The MIT License (MIT)

Copyright (c) 2013 Mark Farver

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/

#include "EEPROM.h"
#include <avr/pgmspace.h>
#include "font5x7.h"

#define shift_latch_bit_pos    7
#define shift_clock_bit_pos    6
#define shift_data_bit_pos     5

#define shift_latch_mask       1 << shift_latch_bit_pos
#define shift_clock_mask       1 << shift_clock_bit_pos
#define shift_data_mask        1 << shift_data_bit_pos

#define MSG_LENGTH_ADDR 0
#define START_OF_MSG_CRC MSG_LENGTH_ADDR + 1
#define START_OF_MSG_ADDR START_OF_MSG_CRC + 4
#define MAX_MSG_LENGTH 255

// last byte of eeprom contains the badge type
#define BADGE_TYPE_ADDR 1023

#define USE_SERIAL

#ifdef USE_SERIAL
unsigned char serial_state;
unsigned long serial_crc;
unsigned long serial_crc2;
unsigned char serial_len;
unsigned char serial_pos;
char serial_message[MAX_MSG_LENGTH+1];
#endif

// how many demo modes do we have?
#define DEMO_COUNT 3

//How many times should we paint the matrix before scrolling it
#define SCROLL_REFRESH 20


boolean reload;
int message_num_cols;
char message[MAX_MSG_LENGTH];

// keeps track of which demo function is being run currently
unsigned char current_demo;
unsigned char demo_speed;

//number columns offset into the message, where screen draw should start
unsigned int current_col;

//keeps track of number of refreshes, counts up to SCROLL_REFRESH
int scroll_count;

unsigned char badge_type;

#define BADGE_TYPE_COUNT 4

const char *default_msg[BADGE_TYPE_COUNT] = {
  "  Speaker",
  "  Staff",
  "  GrrCON",
  "  VIP"
};

const PROGMEM prog_uint32_t crc_table[16] = {
    0x00000000, 0x1db71064, 0x3b6e20c8, 0x26d930ac,
    0x76dc4190, 0x6b6b51f4, 0x4db26158, 0x5005713c,
    0xedb88320, 0xf00f9344, 0xd6d6a3e8, 0xcb61b38c,
    0x9b64c2b0, 0x86d3d2d4, 0xa00ae278, 0xbdbdf21c
};

unsigned long crc_update(unsigned long crc, byte data)
{
    byte tbl_idx;
    tbl_idx = crc ^ (data >> (0 * 4));
    crc = pgm_read_dword_near(crc_table + (tbl_idx & 0x0f)) ^ (crc >> 4);
    tbl_idx = crc ^ (data >> (1 * 4));
    crc = pgm_read_dword_near(crc_table + (tbl_idx & 0x0f)) ^ (crc >> 4);
    return crc;
}

unsigned long crc_string(char *s, unsigned int len)
{
  unsigned long crc = ~0L;
  for(unsigned int i = 0; i < len; ++i)
    crc = crc_update(crc, s[i]);
  crc = ~crc;
  return crc;
}

void store_crc(unsigned int addr, unsigned long crc) {
  for(char i = 0; i < 4; ++i) {
    EEPROM.write(addr + i, ((crc >> (8 * (3-i))) & 0xff));
  }
}

unsigned long load_crc(unsigned int addr) {
  unsigned long crc = 0;
  for(char i = 0; i < 4; ++i) {
    crc |= ((unsigned long) EEPROM.read(addr + i)) << (8 * (3-i));
  }
  return crc;
}

void load_msg() {
  //We need to load the message length
  //The message is stored in a Pascal string in EEPROM
  message_num_cols = EEPROM.read(MSG_LENGTH_ADDR);
  
  // if the EEPROM has not been initialized, we have no message
  if(message_num_cols == 0xFF) {
    message_num_cols = 0;
  }
  
  badge_type = EEPROM.read(BADGE_TYPE_ADDR);
  
  if((message_num_cols == 0) && (badge_type >= BADGE_TYPE_COUNT)) {
    current_demo = 0;
  }
  
  // copy the message from EEPROM (which is slow)
  if(message_num_cols > 0) {
    for(unsigned int i = 0; i < message_num_cols; ++i) {
      message[i] = EEPROM.read(START_OF_MSG_ADDR + i);
    }
    unsigned long msg_crc = crc_string(message, message_num_cols);
    unsigned long stored_crc = load_crc(START_OF_MSG_CRC);

    // if the crc does not match, reject and fall back on defaults
    if(msg_crc != stored_crc) {
      message_num_cols = 0;
    }
  }
  
  // don't combine. if the message failed CRC, we reject it and go to default
  if((message_num_cols == 0) && (badge_type < BADGE_TYPE_COUNT)) {
    while(default_msg[badge_type][message_num_cols]) {
      message[message_num_cols] = default_msg[badge_type][message_num_cols];
      ++message_num_cols;
    }
  }

  message_num_cols *= CHAR_WIDTH;
  current_col = 0;
  scroll_count = 0;
  reload = false;
}

void setup() {                
  pinMode(shift_latch_bit_pos, OUTPUT);     
  pinMode(shift_clock_bit_pos, OUTPUT);     
  pinMode(shift_data_bit_pos, OUTPUT);
  
#ifdef USE_SERIAL
  Serial.begin(9600);
  
  serial_state = 0;
#endif

  current_demo = 0;  
  load_msg();
}

// forward-declare demo routine
void demo();

// the loop routine runs over and over again forever:
void loop() {
  // reload message, if signalled to do so
  if(reload) {
    load_msg();
  }
  
  // startup - show demo
  if(current_demo < DEMO_COUNT) {
    demo();
    return;
  }

  for (int i=0; i < 8; ++i) {
    unsigned int print_col = (current_col + i) % message_num_cols;
    char letter = message[print_col / CHAR_WIDTH];
    if((letter < FIRST_CHAR) || (letter > LAST_CHAR)) {
      letter = FIRST_CHAR;
    }
    unsigned int offset = print_col % CHAR_WIDTH;
    writeCol(i, font5x7[ (letter - FIRST_CHAR) * CHAR_WIDTH + offset]);
  }  

  if ( ++scroll_count > SCROLL_REFRESH ) {
     scroll_count = 0;
     current_col = (current_col + 1) % message_num_cols;
  }
}

#ifdef USE_SERIAL
// Not compatible with Esplora, Leonardo, or Micro
void serialEvent() 
{
  // extremely paranoid protocol:
  // first, look for three 0x03 (ctrl-c)
  // then get the length and crc.
  // then the crc with each byte xored by the length
  // then length bytes of message data.
  
  // to change badge type:
  // three 0x99 followed by the badge type number three times
  
  if (Serial.available() > 0) 
  {
    unsigned char newchar = Serial.read();
    Serial.write(newchar);

    if(0 == serial_state) {
      if(0x03 == newchar) {
        serial_len = 1;
        ++serial_state;
      } else if(0x99 == newchar) {
        serial_state = 100;
        serial_len = 1;
      }
    } else if(1 == serial_state) {
      if(0x03 == newchar) {
        if(++serial_len == 3) {
          ++serial_state;
        }
      } else {
        serial_state = 0;
      }
    } else if(2 == serial_state) {
      serial_len = newchar;
      ++serial_state;
    } else if(3 == serial_state) {
      serial_crc = ((unsigned long)newchar) << 24;
      ++serial_state;
    } else if(4 == serial_state) {
      serial_crc |= ((unsigned long)newchar) << 16;
      ++serial_state;
    } else if(5 == serial_state) {
      serial_crc |= ((unsigned long)newchar) << 8;
      ++serial_state;
    } else if(6 == serial_state) {
      serial_crc |= ((unsigned long)newchar);
      ++serial_state;
    } else if(7 == serial_state) {
      serial_crc2 = ((unsigned long)(newchar ^ serial_len)) << 24;
      ++serial_state;
    } else if(8 == serial_state) {
      serial_crc2 |= ((unsigned long)(newchar ^ serial_len)) << 16;
      ++serial_state;
    } else if(9 == serial_state) {
      serial_crc2 |= ((unsigned long)(newchar ^ serial_len)) << 8;
      ++serial_state;
    } else if(10 == serial_state) {
      serial_crc2 |= ((unsigned long)(newchar ^ serial_len));
      if(serial_crc == serial_crc2) {
        ++serial_state;
        serial_pos = 0;
      } else {
        serial_state = 0;
      }
    } else if(11 == serial_state) {
      serial_message[serial_pos++] = newchar;
      if(serial_pos == serial_len) {
        serial_state = 0;
        unsigned long calccrc = crc_string(serial_message, serial_len);
        if(calccrc == serial_crc) {
          for(unsigned char i = 0; i < serial_len; ++i) {
            EEPROM.write(START_OF_MSG_ADDR + i, serial_message[i]);
          }
          EEPROM.write(MSG_LENGTH_ADDR, serial_len);
          store_crc(START_OF_MSG_CRC, serial_crc);
          Serial.print("Wrote message:");
          serial_message[serial_len] = 0;
          Serial.println(serial_message);
          reload = true;
        }
      }
    } else if(100 == serial_state) {
      if(0x99 == newchar) {
        if(++serial_len == 3) {
          ++serial_state;
        }
      } else {
        serial_state = 0;
      }
    } else if(101 == serial_state) {
      newchar -= '0';
      if(newchar < BADGE_TYPE_COUNT) {
        serial_message[0] = newchar;
        serial_len = 1;
        ++serial_state;
      } else {
        serial_state = 0;
      }
    } else if(102 == serial_state) {
      newchar -= '0';
      if(serial_message[0] == newchar) {
        if(++serial_len == 3) {
          Serial.print("Setting badge type to:");
          Serial.println(default_msg[newchar]);
          EEPROM.write(BADGE_TYPE_ADDR, newchar);
          serial_state = 0;
          reload = true;
        }
      } else {
        serial_state = 0;
      }
    }
  }
}
#endif

void writeCol(unsigned int col, unsigned int data){
  //The selected column must be driven low, and the
  //lowest number column is actually the rightmost
  //to simplify things we reorder it by shifting left
  //so column 0 is on the left.
  write_shift_reg( ~(0x80 >> col) );
  write_shift_reg( data );
  strobe_latch();
}

void strobe_latch() {
  PORTD |= shift_latch_mask;
  PORTD &= ~shift_latch_mask;
}

void write_shift_reg(unsigned int write_me) {
  int i;
  int mask = 0x80;
  for (i=0; i<8; ++i) {
    PORTD &= ~shift_clock_mask;  //Set Clock Low
    if ( write_me & mask ) 
      PORTD |= shift_data_mask;
    else   
      PORTD &= ~shift_data_mask;
    PORTD |= shift_clock_mask;  //Set Clock H
    mask >>= 1;
  }
}

void demo() {
  unsigned char demo_speed = 1;
  
  if(0 == message_num_cols) {
    demo_speed = 10;
  }
  
  if(current_demo == 0) {
    // all LEDs on
	for(char i = 0; i < 8; ++i) {
	  writeCol(i, 0x7F);
	}
	if(++scroll_count > (SCROLL_REFRESH * 5 * demo_speed)) {
	  scroll_count = 0;
	  ++current_demo;
	  current_col = 0;
	}
  } else if(current_demo == 1) {
    // first odd lines, then even lines
    char mask = (0x55 << (current_col % 2)) & 0x7F;
    for(char i = 0; i < 8; ++i) {
      writeCol(i, mask);
    }
    if(++scroll_count > (SCROLL_REFRESH * 2 * demo_speed)) {
      scroll_count = 0;
      ++current_col;
    }
    if(current_col > 9) {
      scroll_count = 0;
      ++current_demo;
      current_col = 0;
    }
  } else if(current_demo == 2) {
    // one row on, starting at the top
    char mask = (0x01 << current_col) & 0x7F;
    for(char i = 0; i < 8; ++i) {
      writeCol(i, mask);
    }
    if(++scroll_count > (SCROLL_REFRESH * 2 * demo_speed)) {
      scroll_count = 0;
      ++current_col;
    }
    if(current_col > 6) {
      scroll_count = 0;
      ++current_demo;
      current_col = 0;
    }
  }

  // and loop around if there is no message
  if((current_demo >= DEMO_COUNT) && (message_num_cols == 0)) {
    current_demo = 0;
  }
}
