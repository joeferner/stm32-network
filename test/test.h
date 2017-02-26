
#ifndef _NETWORK_TEST_H_
#define _NETWORK_TEST_H_

#include <stdio.h>
#include <stdint.h>

#define mu_assert(message, test) \
  do { \
    if (!(test)) { \
      printf("FAIL: assert (%s:%d)\n", __FILE__, __LINE__); \
      return message; \
    } \
  } while (0)

#define mu_assert_equals_int(expected, found) \
  do { \
    if ((expected)!=(found)) { \
      printf("FAIL: expected: %d (0x%x) found: %d (0x%x) (%s:%d)\n", (int)(expected), (int)(expected), (int)(found), (int)(found), __FILE__, __LINE__); \
      return "not equal"; \
    } \
  } while (0)
  
#define mu_assert_equals_byteArray(expected, expectedLen, found, foundLen) \
  do { \
    if ((expectedLen)!=(foundLen)) { \
      printf("FAIL: expected length: %d found: %d (%s:%d)\n", (int)(expectedLen), (int)(foundLen), __FILE__, __LINE__); \
      return "not equal"; \
    } \
    for(int __i = 0; __i < expectedLen; __i++) { \
      if (((expected)[__i])!=((found)[__i])) { \
        printf("FAIL: expected: 0x%02x found: 0x%02x at index %d (%s:%d)\n", (expected)[__i], (found)[__i], __i, __FILE__, __LINE__); \
        return "not equal"; \
      } \
    } \
  } while (0)

#define mu_run_test(test) do { char *message = test(); tests_run++; if (message) return message; } while (0)
  
extern int tests_run;

#endif
