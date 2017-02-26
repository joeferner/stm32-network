
#ifndef _checksum_h_
#define _checksum_h_

#include <stdint.h>

uint16_t Checksum_calculateOnesComplement(uint8_t* p, int len);
uint16_t Checksum_calculateOnesComplement2(uint8_t* p1, int len1, uint8_t* p2, int len2);

#endif

