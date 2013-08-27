/* Basic scrolling Signboard app for the GrrCON badge */

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
}

//offset into the message, where screen draw should start
unsigned int current_col = 12;  

//How many times should we paint the matrix before scrolling it
const int SCROLL_REFRESH = 50;

//keeps track of number of refreshes, counts up to SCROLL_REFRESH
int scroll_count = 0;

// the loop routine runs over and over again forever:
void loop() {
  char letter = ' ';
  unsigned int offset = 0;
  unsigned int i = 0;

  // TODO, seems like we have an off by one error that cuases a brief garbage
  //   column at the end of scroll.  I think it is becuase we are not handling
  //   wrap around correctly.  
  for (i=0; i < 8; ++i) {
    letter = message[(current_col + i) / CHAR_WIDTH];
    offset = (current_col + i) % CHAR_WIDTH;
    writeCol(i, font5x7[ (letter - FIRST_CHAR) * CHAR_WIDTH + offset]);
  }  

  if ( ++scroll_count > SCROLL_REFRESH ) {
     scroll_count = 0;
     current_col = (current_col + 1) % MESSAGE_COLS;
  }
}

void writeCol(unsigned int col, unsigned int data){
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
    
  
