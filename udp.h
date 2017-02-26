
#ifndef _udp_h_
#define _udp_h_

#include <stdint.h>
#include "interface.h"

#pragma pack(push,1)
typedef struct {
  uint16_t sourcePort;
  uint16_t destPort;
  uint16_t length;
  uint16_t checksum;
} UDP_Header;
#pragma pack(pop)

void UDP_initHeader(NetworkInterface* ni, uint16_t sourcePort, uint8_t* destMac, uint32_t destIp, uint16_t destPort);
void UDP_finalize(NetworkInterface* ni, uint16_t dataLength);

#endif
