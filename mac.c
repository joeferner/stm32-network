
#include "mac.h"
#include <string.h>
#include <stdio.h>

uint8_t MAC_BROADCAST[MAC_ADDRESS_SIZE] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};

void MAC_initHeader(NetworkInterface* ni, uint8_t* destMac, uint16_t etherType) {
  MAC_Header* header = (MAC_Header*)ni->buffer;
  memcpy(header->destMac, destMac, MAC_ADDRESS_SIZE);
  memcpy(header->sourceMac, ni->macAddress, MAC_ADDRESS_SIZE);
  header->etherType = htons(etherType);
  ni->bufferEnd = ni->buffer + sizeof(MAC_Header);
}

MAC_Header* MAC_getHeader(NetworkInterface* ni) {
  MAC_Header* header = (MAC_Header*)ni->buffer;
  return header;
}

void MAC_macToString(char* buffer, uint8_t* macAddress) {
  sprintf(
    buffer,
    "%02x:%02x:%02x:%02x:%02x:%02x",
    macAddress[0], macAddress[1], macAddress[2],
    macAddress[3], macAddress[4], macAddress[5]
  );
}
