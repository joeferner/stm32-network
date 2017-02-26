
#ifndef _ipv4_h_
#define _ipv4_h_

#include <stdint.h>
#include "interface.h"

#define IPV4_VERSION 4

#define IPV4_ADDRESS_LENGTH 4

#define IPV4_FLAG_DONT_FRAGMENT  0x02
#define IPV4_FLAG_MORE_FRAGMENTS 0x01

#define IPTOINT32(a0, a1, a2, a3) (  \
  (((uint32_t)(a0) & 0xff) << 0)     \
  | (((uint32_t)(a1) & 0xff) << 8)   \
  | (((uint32_t)(a2) & 0xff) << 16)  \
  | (((uint32_t)(a3) & 0xff) << 24)  \
)

typedef enum {
  IPV4_Protocol_ICMP = 0x01,
  IPV4_Protocol_TCP = 0x06,
  IPV4_Protocol_UDP = 0x11
} IPV4_Protocols;

#pragma pack(push,1)
typedef struct {
  uint8_t ihl: 4;
  uint8_t version: 4;
  uint8_t dscp: 6;
  uint8_t ecn: 2;
  uint16_t totalLength;
  uint16_t identification;
  uint8_t fragmentOffsetHigh: 5;
  uint8_t flags: 3;
  uint8_t fragmentOffsetLow;
  uint8_t ttl;
  uint8_t protocol;
  uint16_t headerChecksum;
  uint32_t sourceIpAddress;
  uint32_t destIpAddress;
} IPV4_Header;
#pragma pack(pop)

void IPV4_initHeader(
  NetworkInterface* ni,
  IPV4_Protocols protocol,
  uint8_t* destMac,
  uint32_t destIpAddress
);
void IPV4_initReplyHeader(NetworkInterface* ni);

void IPV4_ipToString(char* buffer, uint32_t addr);
void IPV4_finalize(NetworkInterface* ni, uint16_t dataLength);
IPV4_Header* IPV4_getHeader(NetworkInterface* ni);

#endif
