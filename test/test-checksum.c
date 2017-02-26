#include "test.h"
#include "../checksum.h"
#include <string.h>

static char* checksum_test_1();

char* checksum_test() {
  mu_run_test(checksum_test_1);
  return NULL;
}

static char* checksum_test_1() {
  uint8_t data0[16];
  uint8_t data1[16];
  uint8_t data2[32];
  
  memset(data0, 0, sizeof(data0));  
  mu_assert_equals_int(0xffff, Checksum_calculateOnesComplement(data0, sizeof(data0)));

  memset(data0, 0xff, sizeof(data0));  
  mu_assert_equals_int(0x0000, Checksum_calculateOnesComplement(data0, sizeof(data0)));

  memset(data0, 0x01, sizeof(data0));  
  mu_assert_equals_int(0xf7f7, Checksum_calculateOnesComplement(data0, sizeof(data0)));

  memset(data0, 0x01, sizeof(data0));  
  memset(data1, 0x02, sizeof(data1));  
  mu_assert_equals_int(0xf7f7, Checksum_calculateOnesComplement(data0, sizeof(data0)));
  mu_assert_equals_int(0xefef, Checksum_calculateOnesComplement(data1, sizeof(data1)));

  memcpy(data2, data0, sizeof(data0));
  memcpy(data2 + sizeof(data0), data1, sizeof(data1));
  mu_assert_equals_int(0xe7e7, Checksum_calculateOnesComplement(data2, sizeof(data2)));
  mu_assert_equals_int(0xe7e7, Checksum_calculateOnesComplement2(data0, sizeof(data0), data1, sizeof(data1)));
  
  return NULL;
}
