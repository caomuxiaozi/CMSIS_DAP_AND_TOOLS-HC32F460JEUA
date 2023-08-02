#include "hex2bin.h"

unsigned char ChartoByte(char c)
{
    if(c-'a'>=0 ) return(c-'a'+10);
    else if(c-'A'>=0 ) return(c-'A'+10);
    else return(c-'0');
}

unsigned char Char2toByte(uint8_t* s)
{
    return ((ChartoByte(*s)<<4) | ChartoByte(*(s+1)));
}
unsigned short Char4toUint16(uint8_t*s){
  return ((Char2toByte(s) << 8)| Char2toByte(s+2));
}
unsigned int Char8toUint32(uint8_t *s){
  return ((Char4toUint16(s)<<16) | Char4toUint16(s+4));
}

  