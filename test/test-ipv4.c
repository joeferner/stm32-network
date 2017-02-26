
#include "test.h"
#include "../ipv4.h"
#include "../mac.h"

extern NetworkInterface ni;

static char* ipv4_test_packet();

char* ipv4_test() {
  mu_run_test(ipv4_test_packet);
  return NULL;
}

static char* ipv4_test_packet() {
  uint8_t destMac[] = { 0x11, 0x12, 0x13, 0x14, 0x15, 0x16 };
  uint8_t expectedBytes[] = {
    0x45, // ihl/version
    0x00, // dscp/ecn
    0x03, 0xfc, // total length
    0x00, 0x00, // identification
    0x40, // fragmentOffsetHigh/flags
    0x00, // fragmentOffsetLow
    0xff, // ttl
    0x11, // protocol
    0xc1, 0x6f, // headerChecksum
    0x64, 0x00, 0xa8, 0xc0, // sourceIpAddress
    0x01, 0x00, 0xa8, 0xc0 // destIpAddress
  };

  uint32_t ipPacketOffset = sizeof(MAC_Header);

  ni.ipAddress = IPTOINT32(192,168,0,100);
  IPV4_initHeader(&ni, IPV4_Protocol_UDP, destMac, IPTOINT32(192,168,0,1));
  IPV4_finalize(&ni, 1000);
  mu_assert_equals_byteArray(
    expectedBytes,
    sizeof(expectedBytes),
    ni.buffer + ipPacketOffset,
    ni.bufferEnd - ni.buffer - ipPacketOffset
  );
  return NULL;
}
