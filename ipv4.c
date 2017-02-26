
#include <string.h>
#include <stdio.h>
#include "ipv4.h"
#include "mac.h"
#include "checksum.h"

void IPV4_initHeader(
  NetworkInterface* ni,
  IPV4_Protocols protocol,
  uint8_t* destMac,
  uint32_t destIpAddress
) {
  MAC_initHeader(ni, destMac, MAC_ETHER_TYPE_IPV4);

  IPV4_Header* header = (IPV4_Header*)(ni->buffer + sizeof(MAC_Header));
  header->version = IPV4_VERSION;
  header->ihl = 5;
  header->dscp = 0x00;
  header->ecn = 0x00;
  header->identification = 0x0000;
  header->flags = IPV4_FLAG_DONT_FRAGMENT;
  header->fragmentOffsetLow = 0x00;
  header->fragmentOffsetHigh = 0x00;
  header->ttl = 0xff;
  header->protocol = protocol;
  header->headerChecksum = 0x0000;
  header->sourceIpAddress = htonl(ni->ipAddress);
  header->destIpAddress = htonl(destIpAddress);
  ni->bufferEnd += sizeof(IPV4_Header);
}

void IPV4_initReplyHeader(NetworkInterface* ni) {
  uint8_t destMac[MAC_ADDRESS_SIZE];
  uint32_t destIp;

  MAC_Header* macHeader = MAC_getHeader(ni);
  memcpy(destMac, macHeader->sourceMac, MAC_ADDRESS_SIZE);

  IPV4_Header* ipv4Header = IPV4_getHeader(ni);
  destIp = ntohl(ipv4Header->sourceIpAddress);

  IPV4_initHeader(ni, ipv4Header->protocol, destMac, destIp);
}

void IPV4_finalize(NetworkInterface* ni, uint16_t dataLength) {
  IPV4_Header* header = (IPV4_Header*)(ni->buffer + sizeof(MAC_Header));
  header->totalLength = htons(dataLength + sizeof(IPV4_Header));
  header->headerChecksum = 0x0000;
  header->headerChecksum = htons(Checksum_calculateOnesComplement((uint8_t*)header, sizeof(IPV4_Header)));
}

IPV4_Header* IPV4_getHeader(NetworkInterface* ni) {
  return (IPV4_Header*)(ni->buffer + sizeof(MAC_Header));
}

void IPV4_ipToString(char* buffer, uint32_t addr) {
  sprintf(
    buffer,
    "%d.%d.%d.%d",
    (int)((addr >> 24) & 0xff),
    (int)((addr >> 16) & 0xff),
    (int)((addr >> 8) & 0xff),
    (int)((addr >> 0) & 0xff)
  );
}
