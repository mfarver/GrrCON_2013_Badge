// some C-ish pseudo-code for writing to the badges

void serial_write(unsigned char c);

void setmessage(char *message)
{
  unsigned char len = strlen(message);
  unsigned int crc = crc32(message, len);

  // nine bytes of unimportant data to clear the state
  for(i = 0; i < 9; ++i)
    serial_write('a');

  // three bytes of command char
  for(i = 0; i < 3; ++i)
     serial_write(0x03);

  // the length byte
  serial_write(len);

  // the crc, network byte order
  for(i = 0; i < 4; ++i)
    serial_write( (crc >> (8 * (3-i))) & 0xff);

  // the crc^len, network byte order
  for(i = 0; i < 4; ++i)
    serial_write(len ^ ((crc >> (8 * (3-i))) & 0xff) );

  // the message data itself
  for(i = 0; i < len; ++i)
    serial_write(message[i]);

  // the badge will echo back every byte it gets, but upon
  // a successful programming command, it will send back:
  // Wrote message:
  // [message text]
  
}

void setbadgetype(int type)
{
  // valid badge types:
  // 0 - Speaker
  // 1 - Staff
  // 2 - GrrCON
  // 3 - VIP

  if((type < 0) || (type > 3))
    return;

  // write 9 bytes to clear out of a "write message" state
  for(i = 0; i < 9; ++i)
    serial_write('a');

  // the badge type command character three times
  for(i = 0; i < 3; ++i)
    serial_write(0x01);

  // the badge type three times, as ascii
  for(i = 0; i < 3; ++i)
    serial_write(((char)type) + '0');

  // the badge will echo back everything it gets, but upon successful
  // badge type command, it will send back:
  // Setting badge type to:  [typestring]
  // where the typestring is "Speaker", "Staff", "GrrCON", or "VIP"
}

