
#include "interface.h"
#include "mac.h"
#include "ipv4.h"
#include "udp.h"
#include "tcp.h"
#include "dhcp.h"
#include "arp.h"
#include "icmp.h"
#include <string.h>
#include <stdio.h>
#include <utils/utils.h>
#include <platform_config.h>

#ifdef NETWORK_INTERFACE_DEBUG
#define NETWORK_INTERFACE_DEBUG_OUT(format, ...) printf("%s:%d: ni: " format, __FILE__, __LINE__, ##__VA_ARGS__)
#else
#define NETWORK_INTERFACE_DEBUG_OUT(format, ...)
#endif

void _NetworkInterface_handleIPv4(NetworkInterface* ni, MAC_Header* macHeader);
void _NetworkInterface_handleIPv4MessageDirectedAtMac(NetworkInterface* ni);
void _NetworkInterface_handleIPv4UdpMessageDirectedAtMac(NetworkInterface* ni, UDP_Header* udpHeader);
void _NetworkInterface_handleArp(NetworkInterface* ni, MAC_Header* macHeader);

void NetworkInterface_init(
  NetworkInterface* ni,
  uint8_t* macAddress,
  NetworkInterface_read* receive,
  NetworkInterface_send* send
) {
  memset(ni, 0, sizeof(NetworkInterface));
  memcpy(ni->macAddress, macAddress, MAC_ADDRESS_SIZE);
  for (int i = 0; i < ARP_TABLE_SIZE; i++) {
    ni->arpTable[i].valid = false;
  }
  ni->receive = receive;
  ni->send = send;
}

void NetworkInterface_sendBuffer(NetworkInterface* ni) {
  NETWORK_INTERFACE_DEBUG_OUT("send: %d\n", (int)(ni->bufferEnd - ni->buffer));
#ifdef NETWORK_INTERFACE_DEBUG_PACKETS
  printMemory(ni->buffer, ni->bufferEnd - ni->buffer);
#endif
  ni->send(ni, ni->buffer, ni->bufferEnd - ni->buffer);
}

void NetworkInterface_tick(NetworkInterface* ni) {
  memset(ni->buffer, 0, NETWORK_INTERFACE_BUFFER_SIZE);
  int count = ni->receive(ni, ni->buffer, NETWORK_INTERFACE_BUFFER_SIZE);
  if (count == 0) {
    return;
  }
  NETWORK_INTERFACE_DEBUG_OUT("receive count: %d\n", count);
#ifdef NETWORK_INTERFACE_DEBUG_PACKETS
  printMemory(ni->buffer, count);
#endif
  ni->bufferEnd = ni->buffer + count;

  MAC_Header* macHeader = (MAC_Header*)ni->buffer;
  uint16_t etherType = ntohs(macHeader->etherType);
  if (etherType == MAC_ETHER_TYPE_IPV4) {
    _NetworkInterface_handleIPv4(ni, macHeader);
  } else if (etherType == MAC_ETHER_TYPE_ARP) {
    _NetworkInterface_handleArp(ni, macHeader);
  } else {
    NETWORK_INTERFACE_DEBUG_OUT("unhandled ether type: 0x%02x\n", etherType);
  }
}

void _NetworkInterface_handleIPv4(NetworkInterface* ni, MAC_Header* macHeader) {
  if (memcmp(macHeader->destMac, ni->macAddress, MAC_ADDRESS_SIZE) == 0) {
    _NetworkInterface_handleIPv4MessageDirectedAtMac(ni);
  }
}

void _NetworkInterface_handleIPv4MessageDirectedAtMac(NetworkInterface* ni) {
  IPV4_Header* ipv4Header = (IPV4_Header*)(ni->buffer + sizeof(MAC_Header));
  uint32_t len = (uint32_t)ipv4Header->ihl * 4;
  uint8_t* p = ((uint8_t*)ipv4Header) + len;
  NETWORK_INTERFACE_DEBUG_OUT("_NetworkInterface_handleIPv4MessageDirectedAtMac(protocol: %d)\n", ipv4Header->protocol);
  if (ipv4Header->protocol == IPV4_Protocol_ICMP) {
    ICMP_handlePacket(ni, (ICMP_Header*)p);
  } else if (ipv4Header->protocol == IPV4_Protocol_UDP) {
    _NetworkInterface_handleIPv4UdpMessageDirectedAtMac(ni, (UDP_Header*)p);
  } else if (ipv4Header->protocol == IPV4_Protocol_TCP) {
    TCP_handlePacket(ni, (TCP_Header*)p);
  } else {
    NETWORK_INTERFACE_DEBUG_OUT("unhandled protocol: 0x%02x\n", ipv4Header->protocol);
  }
}

void _NetworkInterface_handleIPv4UdpMessageDirectedAtMac(NetworkInterface* ni, UDP_Header* udpHeader) {
  uint8_t* p = ((uint8_t*)udpHeader) + sizeof(UDP_Header);
  uint16_t sourcePort = ntohs(udpHeader->sourcePort);
  uint16_t destPort = ntohs(udpHeader->destPort);
  if (sourcePort == DHCP_UDP_DEST_PORT && destPort == DHCP_UDP_SOURCE_PORT) {
    DHCP_handleDhcpResponse(ni, (DHCP_Header*)p);
  }
}

void _NetworkInterface_handleArp(NetworkInterface* ni, MAC_Header* macHeader) {
  ARP_Packet* arpPacket = (ARP_Packet*)(ni->buffer + sizeof(MAC_Header));
  ARP_handleArpPacket(ni, arpPacket);
}

void NetworkInterface_addArpTableEntry(NetworkInterface* ni, uint8_t* macAddress, uint32_t ipAddress) {
  NetworkInterfaceArpTableEntry* oldestEntry = NULL;
  NetworkInterfaceArpTableEntry* entry = NULL;

  for (int i = 0; i < ARP_TABLE_SIZE; i++) {
    if (ni->arpTable[i].valid) {
      if (oldestEntry == NULL) {
        oldestEntry = &ni->arpTable[i];
      } else if (ni->arpTable[i].createTime < oldestEntry->createTime) {
        oldestEntry = &ni->arpTable[i];
      }
    } else {
      entry = &ni->arpTable[i];
      break;
    }
  }

  if (entry == NULL) {
    entry = oldestEntry;
  }

  entry->valid = true;
  entry->createTime = HAL_GetTick();
  memcpy(entry->macAddress, macAddress, MAC_ADDRESS_SIZE);
  entry->ipAddress = ipAddress;
}
