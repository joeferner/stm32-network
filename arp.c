
#include "arp.h"
#include "ipv4.h"
#include <string.h>
#include "platform_config.h"

#ifdef ARP_DEBUG
#define ARP_DEBUG_OUT(format, ...) printf("%s:%d: ARP: " format, __FILE__, __LINE__, ##__VA_ARGS__)
#else
#define ARP_DEBUG_OUT(format, ...)
#endif

void _ARP_handleRequest(NetworkInterface* ni, ARP_Packet* arpPacket);
void _ARP_sendReply(NetworkInterface* ni, uint8_t* destMac, uint32_t destIp);
void _ARP_handleReply(NetworkInterface* ni, ARP_Packet* arpPacket);

void ARP_handleArpPacket(NetworkInterface* ni, ARP_Packet* arpPacket) {
  ARP_DEBUG_OUT("ARP_handleArpPacket\n");
  if (ntohs(arpPacket->hardwareType) != ARP_HTYPE_ETHERNET) {
    return;
  }
  if (ntohs(arpPacket->protocolType) != MAC_ETHER_TYPE_IPV4) {
    return;
  }
  if (arpPacket->hardwareAddressLength != MAC_ADDRESS_SIZE) {
    return;
  }
  if (arpPacket->protocolAddressLength != IPV4_ADDRESS_LENGTH) {
    return;
  }

  uint16_t op = ntohs(arpPacket->operation);
  if (op == ARP_OP_REQUEST) {
    _ARP_handleRequest(ni, arpPacket);
  } else if (op == ARP_OP_REPLY) {
    _ARP_handleReply(ni, arpPacket);
  } else {
    ARP_DEBUG_OUT("invalid op: 0x%04x\n", op);
  }
}

void _ARP_handleRequest(NetworkInterface* ni, ARP_Packet* arpPacket) {
  if (ni->ipAddress != ntohl(arpPacket->targetProtocolAddress)) {
    ARP_DEBUG_OUT("handleRequest ip missmatch %08lx != %08lx\n", ni->ipAddress, ntohl(arpPacket->targetProtocolAddress));
    return;
  }
  ARP_DEBUG_OUT("handleRequest\n");
  uint8_t* destMac = arpPacket->senderHardwareAddress;
  uint32_t destIp = ntohl(arpPacket->senderProtocolAddress);
  _ARP_sendReply(ni, destMac, destIp);
}

void _ARP_sendReply(NetworkInterface* ni, uint8_t* destMac, uint32_t destIp) {
  ARP_DEBUG_OUT("sendReply\n");
  MAC_initHeader(ni, destMac, MAC_ETHER_TYPE_ARP);

  ARP_Packet* responseArpPacket = (ARP_Packet*)ni->bufferEnd;
  responseArpPacket->hardwareType = htons(ARP_HTYPE_ETHERNET);
  responseArpPacket->protocolType = htons(MAC_ETHER_TYPE_IPV4);
  responseArpPacket->hardwareAddressLength = MAC_ADDRESS_SIZE;
  responseArpPacket->protocolAddressLength = IPV4_ADDRESS_LENGTH;
  responseArpPacket->operation = htons(ARP_OP_REPLY);
  memcpy(responseArpPacket->senderHardwareAddress, ni->macAddress, MAC_ADDRESS_SIZE);
  responseArpPacket->senderProtocolAddress = htonl(ni->ipAddress);
  memcpy(responseArpPacket->targetHardwareAddress, destMac, MAC_ADDRESS_SIZE);
  responseArpPacket->targetProtocolAddress = htonl(destIp);
  ni->bufferEnd += sizeof(ARP_Packet);

  NetworkInterface_sendBuffer(ni);
}

void ARP_sendRequest(NetworkInterface* ni, uint32_t requestedIpAddress) {
  ARP_DEBUG_OUT("sendRequest\n");
  MAC_initHeader(ni, MAC_BROADCAST, MAC_ETHER_TYPE_ARP);

  ARP_Packet* requestArpPacket = (ARP_Packet*)ni->bufferEnd;
  requestArpPacket->hardwareType = htons(ARP_HTYPE_ETHERNET);
  requestArpPacket->protocolType = htons(MAC_ETHER_TYPE_IPV4);
  requestArpPacket->hardwareAddressLength = MAC_ADDRESS_SIZE;
  requestArpPacket->protocolAddressLength = IPV4_ADDRESS_LENGTH;
  requestArpPacket->operation = htons(ARP_OP_REQUEST);
  memcpy(requestArpPacket->senderHardwareAddress, ni->macAddress, MAC_ADDRESS_SIZE);
  requestArpPacket->senderProtocolAddress = htonl(ni->ipAddress);
  memset(requestArpPacket->targetHardwareAddress, 0x00, MAC_ADDRESS_SIZE);
  requestArpPacket->targetProtocolAddress = htonl(requestedIpAddress);
  ni->bufferEnd += sizeof(ARP_Packet);

  NetworkInterface_sendBuffer(ni);
}

void _ARP_handleReply(NetworkInterface* ni, ARP_Packet* arpPacket) {
  if (memcmp(arpPacket->targetHardwareAddress, ni->macAddress, MAC_ADDRESS_SIZE) != 0) {
    return;
  }

  ARP_DEBUG_OUT("handleReply\n");
  NetworkInterface_addArpTableEntry(ni, arpPacket->senderHardwareAddress, ntohl(arpPacket->senderProtocolAddress));
}
