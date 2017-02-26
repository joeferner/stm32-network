
#ifndef _ICMP_H_
#define _ICMP_H_

#include <stdint.h>
#include "interface.h"

#define ICMP_TYPE_REPLY    0
#define ICMP_TYPE_REQUEST  8

#pragma pack(push,1)
typedef struct {
  uint8_t type;
  uint8_t code;
  uint16_t checksum;
} ICMP_Header;

typedef struct {
  ICMP_Header header;
  uint16_t identifier;
  uint16_t sequenceNumber;
} ICMP_Echo;

#pragma pack(pop)

void ICMP_handlePacket(NetworkInterface* ni, ICMP_Header* header);

#endif

