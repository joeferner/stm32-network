
#include "test.h"
#include "../ipv4.h"
#include "../udp.h"
#include "../mac.h"

extern NetworkInterface ni;

static char* udp_test_packet();

char* udp_test() {
  mu_run_test(udp_test_packet);
  return NULL;
}

static char* udp_test_packet() {
  uint8_t destMac[] = { 0x11, 0x12, 0x13, 0x14, 0x15, 0x16 };
  uint8_t expectedBytes[] = {
    0x03, 0xe8, // sourcePort
    0x07, 0xd0, // destPort
    0x03, 0xf0, // length
    0x00, 0x00 // checksum
  };

  uint32_t udpPacketOffset = sizeof(MAC_Header) + sizeof(IPV4_Header);

  UDP_initHeader(&ni, 1000, destMac, IPTOINT32(192,168,0,1), 2000);
  UDP_finalize(&ni, 1000);
  mu_assert_equals_byteArray(
    expectedBytes,
    sizeof(expectedBytes),
    ni.buffer + udpPacketOffset,
    ni.bufferEnd - ni.buffer - udpPacketOffset
  );
  return NULL;
}
