
#ifndef _mac_h_
#define _mac_h_

#include <stdint.h>
#include "interface.h"

#define MAC_ADDRESS_SIZE 6

#define MAC_ETHER_TYPE_IPV4  0x0800
#define MAC_ETHER_TYPE_ARP   0x0806

extern uint8_t MAC_BROADCAST[MAC_ADDRESS_SIZE];

#pragma pack(push,1)
typedef struct {
  uint8_t destMac[MAC_ADDRESS_SIZE];
  uint8_t sourceMac[MAC_ADDRESS_SIZE];
  uint16_t etherType;
} MAC_Header;
#pragma pack(pop)

void MAC_macToString(char* buffer, uint8_t* macAddress);
void MAC_initHeader(NetworkInterface* ni, uint8_t* destMac, uint16_t etherType);
MAC_Header* MAC_getHeader(NetworkInterface* ni);

#endif
