
#include <stdio.h>
#include "icmp.h"
#include "ipv4.h"
#include "checksum.h"
#include "platform_config.h"

#ifdef ICMP_DEBUG
#define ICMP_DEBUG_OUT(format, ...) printf("%s:%d: ICMP: " format, __FILE__, __LINE__, ##__VA_ARGS__)
#else
#define ICMP_DEBUG_OUT(format, ...)
#endif

void _ICMP_handleRequestPacket(NetworkInterface* ni, ICMP_Echo* echoPacket);

void ICMP_handlePacket(NetworkInterface* ni, ICMP_Header* header) {
  ICMP_DEBUG_OUT("handlePacket 0x02x\n", header->type);
  if (header->type == ICMP_TYPE_REQUEST) {
    _ICMP_handleRequestPacket(ni, (ICMP_Echo*)header);
  } else {
    ICMP_DEBUG_OUT("Unhandled ICMP package: 0x%02x\n", header->type);
  }
}

void _ICMP_handleRequestPacket(NetworkInterface* ni, ICMP_Echo* echoPacket) {
  ICMP_DEBUG_OUT("handleRequestPacket\n");
  uint16_t icmpLength = ni->bufferEnd - (uint8_t*)echoPacket;
  IPV4_initReplyHeader(ni);

  echoPacket->header.type = ICMP_TYPE_REPLY;
  echoPacket->header.code = 0;
  echoPacket->header.checksum = 0;
  echoPacket->header.checksum = htons(Checksum_calculateOnesComplement((uint8_t*)echoPacket, icmpLength));
  ni->bufferEnd += icmpLength;

  IPV4_finalize(ni, icmpLength);
  NetworkInterface_sendBuffer(ni);
}

