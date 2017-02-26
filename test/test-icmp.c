
#include <string.h>
#include "test.h"
#include "../icmp.h"
#include "../ipv4.h"
#include "../mac.h"

extern NetworkInterface ni;

static char* icmp_test_packet();
extern uint8_t sendBuffer[5000];
extern uint32_t sendBufferLength;

char* icmp_test() {
  mu_run_test(icmp_test_packet);
  return NULL;
}

static char* icmp_test_packet() {
  uint8_t packetBytes[] = {
    0x08, // type
    0x00, // code
    0x00, 0x00, // checksum
    0x01, 0x02, // identifier
    0x03, 0x04, // sequenceNumber
    0x05, 0x06, 0x07 // data
  };

  uint8_t expectedBytes[] = {
    0x00, // type
    0x00, // code
    0xef, 0xf3, // checksum
    0x01, 0x02, // identifier
    0x03, 0x04, // sequenceNumber
    0x05, 0x06, 0x07 // data
  };

  uint32_t icmpPacketOffset = sizeof(MAC_Header) + sizeof(IPV4_Header);

  memcpy(ni.buffer + icmpPacketOffset, packetBytes, sizeof(packetBytes));
  ni.bufferEnd = ni.buffer + icmpPacketOffset + sizeof(packetBytes);
  
  ICMP_handlePacket(&ni, (ICMP_Header*)(ni.buffer + icmpPacketOffset));
  mu_assert_equals_byteArray(
    expectedBytes,
    sizeof(expectedBytes),
    sendBuffer + icmpPacketOffset,
    sendBufferLength - icmpPacketOffset
  );
  return NULL;
}
