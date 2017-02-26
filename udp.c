
#include "udp.h"
#include "mac.h"
#include "ipv4.h"

void UDP_initHeader(NetworkInterface* ni, uint16_t sourcePort, uint8_t* destMac, uint32_t destIp, uint16_t destPort) {
  IPV4_initHeader(ni, IPV4_Protocol_UDP, destMac, destIp);

  UDP_Header* header = (UDP_Header*)ni->bufferEnd;
  header->sourcePort = htons(sourcePort);
  header->destPort = htons(destPort);
  header->length = 0x0000;
  header->checksum = 0x0000;
  ni->bufferEnd += sizeof(UDP_Header);
}

void UDP_finalize(NetworkInterface* ni, uint16_t dataLength) {
  UDP_Header* header = (UDP_Header*)(ni->buffer + sizeof(MAC_Header) + sizeof(IPV4_Header));
  header->length = htons(dataLength + sizeof(UDP_Header));
  IPV4_finalize(ni, dataLength + sizeof(UDP_Header));
}
