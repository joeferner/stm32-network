
#ifndef _arp_h_
#define _arp_h_

#include "mac.h"

#define ARP_HTYPE_ETHERNET 0x0001

#define ARP_OP_REQUEST 1
#define ARP_OP_REPLY   2

#pragma pack(push,1)
typedef struct {
  uint16_t hardwareType;
  uint16_t protocolType;
  uint8_t hardwareAddressLength;
  uint8_t protocolAddressLength;
  uint16_t operation;
  uint8_t senderHardwareAddress[MAC_ADDRESS_SIZE];
  uint32_t senderProtocolAddress;
  uint8_t targetHardwareAddress[MAC_ADDRESS_SIZE];
  uint32_t targetProtocolAddress;
} ARP_Packet;
#pragma pack(pop)

void ARP_handleArpPacket(NetworkInterface* ni, ARP_Packet* arpPacket);
void ARP_sendRequest(NetworkInterface* ni, uint32_t requestedIpAddress);

#endif

