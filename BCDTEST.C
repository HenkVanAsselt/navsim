#include <stdio.h>
#include <serial.h>

main()
{
  unsigned char c;

  s_baudrate(9600);
  s_databits(8);
  s_stopbits(1);
  s_parity('n');

  init_serial();

  while(TRUE)
  {
    if (s_getc(&c))
      printf("%02x",c);
  }
}