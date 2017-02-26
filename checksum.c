
#include "checksum.h"

uint16_t Checksum_calculateOnesComplement(uint8_t* p, int len) {
  int32_t sum = 0;  /* assume 32 bit long, 16 bit short */

  while (len > 1) {
    uint32_t p0 = *p;
    uint32_t p1 = *(p + 1);
    sum += (uint16_t) ((p0 << 8) | p1);
    len -= 2;
    p += 2;
  }

  if (len) {
    uint32_t p0 = *p;
    sum += p0 << 8;
  }

  while (sum >> 16) {
    sum = (sum & 0xFFFF) + (sum >> 16);
  }

  return ~((uint16_t)sum);
}

uint16_t Checksum_calculateOnesComplement2(uint8_t* data1, int len1, uint8_t* data2, int len2) {
  int32_t sum = 0;  /* assume 32 bit long, 16 bit short */

  while (len1 + len2 > 1) {
    uint32_t p0;
    if (len1 > 0) {
      p0 = *data1;
      len1--;
      data1++;
    } else {
      p0 = *data2;
      len2--;
      data2++;
    }
    uint32_t p1;
    if (len1 > 0) {
      p1 = *data1;
      len1--;
      data1++;
    } else {
      p1 = *data2;
      len2--;
      data2++;
    }
    sum += (uint16_t) ((p0 << 8) | p1);
  }

  if (len1 + len2) {
    uint32_t p0;
    if (len1 > 0) {
      p0 = *data1;
      len1--;
      data1++;
    } else {
      p0 = *data2;
      len2--;
      data2++;
    }
    sum += p0 << 8;
  }

  while (sum >> 16) {
    sum = (sum & 0xFFFF) + (sum >> 16);
  }

  return ~((uint16_t)sum);
}
