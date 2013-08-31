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

#include "font5x7.h"

int shift_latch = 7;
int shift_clock = 6;
int data = 5;

const char message[] = "  GR Makers  ";
const int MESSAGE_COLS = CHAR_WIDTH * 12;

// the setup routine runs once when you press reset:
void setup() {                
  // initialize the digital pin as an output.
  pinMode(shift_latch, OUTPUT);     
  pinMode(shift_clock, OUTPUT);     
  pinMode(data, OUTPUT);   
  CLKPR = 0x80;    // Tell the AtMega we want to change the system clock
  CLKPR = 0x03;    // 1/8 prescaler = 2mhz for a 16MHz crystal  
}

//offset into the message, where screen draw should start
unsigned int current_col = 0;  

//How many times should we paint the matrix before scrolling it
const int SCROLL_REFRESH = 5;

//keeps track of number of refreshes, counts up to SCROLL_REFRESH
int scroll_count = 0;

// the loop routine runs over and over again forever:
void loop() {
  char letter = ' ';
  unsigned int offset = 0;
  unsigned int print_col;
  unsigned int i = 0;

  for (i=0; i < 8; ++i) {
    print_col = (current_col + i) % MESSAGE_COLS;
    letter = message[print_col / CHAR_WIDTH];
    offset = print_col % CHAR_WIDTH;
    writeCol(i, font5x7[ (letter - FIRST_CHAR) * CHAR_WIDTH + offset]);
  }  

  if ( ++scroll_count > SCROLL_REFRESH ) {
     scroll_count = 0;
     current_col = (current_col + 1) % MESSAGE_COLS;
  }
}

void writeCol(unsigned int col, unsigned int data){
  //The selected column must be driven low, and the
  //lowest number column is actually the rightmost
  //to simply things we reorder it by shifting left
  //so column 0 is on the left.
  write_shift_reg( ~(0x80 >> col) );
  write_shift_reg( data );
  strobe_pin(shift_latch);
}

void strobe_pin(unsigned int pin) {
    digitalWrite(pin, HIGH);
    digitalWrite(pin, LOW);
}

void write_shift_reg(unsigned int write_me) {
  int i;
  for (i =7; i >=0; --i) {
    digitalWrite(shift_clock, LOW);
    digitalWrite(data, (write_me >> i) & 0x01);
    digitalWrite(shift_clock, HIGH);
  }
}
    
  
