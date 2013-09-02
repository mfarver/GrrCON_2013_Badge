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
#include "font5x7.h"

#define shift_latch_bit_pos    7
#define shift_clock_bit_pos    6
#define shift_data_bit_pos     5

#define shift_latch_mask       1 << shift_latch_bit_pos
#define shift_clock_mask       1 << shift_clock_bit_pos
#define shift_data_mask        1 << shift_data_bit_pos

#define MSG_LENGTH_ADDR 0
#define START_OF_MSG_ADDR MSG_LENGTH_ADDR + 1

int message_num_cols = 0;

//number columns offset into the message, where screen draw should start
unsigned int current_col = 0;  

//How many times should we paint the matrix before scrolling it
const int SCROLL_REFRESH = 20;

//keeps track of number of refreshes, counts up to SCROLL_REFRESH
int scroll_count = 0;

char letter = ' ';
unsigned int offset = 0;
unsigned int print_col;
unsigned int i = 0;
unsigned int romcur = START_OF_MSG_ADDR;


void setup() {                
  pinMode(shift_latch_bit_pos, OUTPUT);     
  pinMode(shift_clock_bit_pos, OUTPUT);     
  pinMode(shift_data_bit_pos, OUTPUT);  
  Serial.begin(9600);
  
  //We need to load the message length
  //The message is stored in a Pascal string in EEPROM
  message_num_cols = EEPROM.read(MSG_LENGTH_ADDR) * CHAR_WIDTH;
}

// the loop routine runs over and over again forever:
void loop() {
  for (i=0; i < 8; ++i) {
    print_col = (current_col + i) % message_num_cols;
    letter = EEPROM.read(print_col / CHAR_WIDTH + START_OF_MSG_ADDR); // TODO: buffer this
    offset = print_col % CHAR_WIDTH;
    writeCol(i, font5x7[ (letter - FIRST_CHAR) * CHAR_WIDTH + offset]);
  }  

  if ( ++scroll_count > SCROLL_REFRESH ) {
     scroll_count = 0;
     current_col = (current_col + 1) % message_num_cols;
  }
}

// Not compatible with Esplora, Leonardo, or Micro
void serialEvent() 
{
  if (Serial.available() > 0) 
  {
    unsigned char newchar = Serial.read();
    Serial.write(newchar);
    if (newchar == '\n' || newchar == '\0')
    {
      int msg_len = romcur - START_OF_MSG_ADDR - 1 -1;
      message_num_cols = msg_len * CHAR_WIDTH;
      EEPROM.write(MSG_LENGTH_ADDR, msg_len);
      romcur = START_OF_MSG_ADDR;
    }
    else if (newchar == '\r') // Windows only, immediately followed by an \n
    {
      // pass
    }
    else
    {
      EEPROM.write(romcur++, newchar);
    }
  }
}

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
    
  
