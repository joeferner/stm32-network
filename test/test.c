
#include "test.h"
#include <stdio.h>
#include <string.h>
#include "../interface.h"

static char* all_tests();
extern char* mac_test();
extern char* ipv4_test();
extern char* udp_test();
extern char* dhcp_test();
extern char* icmp_test();
extern char* tcp_test();
extern char* checksum_test();

int tests_run;

uint8_t MAC_ADDRESS[6] = { 0x01, 0x02, 0x03, 0x04, 0x05, 0x06 };

NetworkInterface ni;
uint8_t readBuffer[5000];
uint32_t readBufferLength;
uint8_t sendBuffer[5000];
uint32_t sendBufferLength;

uint32_t ni_read(NetworkInterface* nic, uint8_t* buffer, uint32_t len);
uint32_t ni_send(NetworkInterface* nic, uint8_t* buffer, uint32_t len);

int main(int argc, char* argv[]) {
  NetworkInterface_init(&ni, MAC_ADDRESS, ni_read, ni_send);

  printf("-------- BEGIN TEST --------\n");
  char* result = all_tests();
  if (result != 0) {
    printf("%s\n", result);
  } else {
    printf("ALL TESTS PASSED\n");
  }
  printf("Tests run: %d\n", tests_run);

  return result != 0;
}

static char* all_tests() {
  mu_run_test(checksum_test);
  mu_run_test(mac_test);
  mu_run_test(ipv4_test);
  mu_run_test(udp_test);
  mu_run_test(dhcp_test);
  mu_run_test(icmp_test);
  mu_run_test(tcp_test);
  return 0;
}

uint32_t ni_read(NetworkInterface* nic, uint8_t* buffer, uint32_t len) {
  memcpy(buffer, readBuffer, readBufferLength);
  return readBufferLength;
}

uint32_t ni_send(NetworkInterface* nic, uint8_t* buffer, uint32_t len) {
  memcpy(sendBuffer, buffer, len);
  sendBufferLength = len;
  return len;
}

void printMemory(uint8_t* buffer, uint32_t length) {
  uint8_t col;
  for (uint32_t offset = 0; offset < length; offset += 16) {
    printf("%08X ", offset);
    for (col = 0; col < 16; col++) {
      if (offset + col < length) {
        printf("%02X ", buffer[offset + col]);
      } else {
        printf("   ");
      }
    }
    for (col = 0; col < 16; col++) {
      if (offset + col < length) {
        char ch = buffer[offset + col];
        if (ch >= '.' && ch <= '~') {
          printf("%c", ch);
        } else {
          printf(".");
        }
      } else {
        printf(" ");
      }
    }
    printf("\n");
  }
}

