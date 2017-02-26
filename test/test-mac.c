
#include "test.h"
#include "../mac.h"

extern NetworkInterface ni;

static char* mac_test_packet();

char* mac_test() {
  mu_run_test(mac_test_packet);
  return NULL;
}

static char* mac_test_packet() {
  uint8_t destMac[] = { 0x11, 0x12, 0x13, 0x14, 0x15, 0x16 };
  uint8_t expectedBytes[] = {
    0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x08, 0x00
  };

  MAC_initHeader(&ni, destMac, MAC_ETHER_TYPE_IPV4);
  mu_assert_equals_byteArray(
    expectedBytes,
    sizeof(expectedBytes),
    ni.buffer,
    ni.bufferEnd - ni.buffer
  );
  return NULL;
}
