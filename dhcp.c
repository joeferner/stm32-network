
#include "dhcp.h"
#include "ipv4.h"
#include "udp.h"
#include <string.h>
#include <stdio.h>
#include "platform_config.h"

#ifdef DHCP_DEBUG
#define DHCP_DEBUG_OUT(format, ...) printf("%s:%d: DHCP: " format, __FILE__, __LINE__, ##__VA_ARGS__)
#else
#define DHCP_DEBUG_OUT(format, ...)
#endif

uint8_t DHCP_DEST_MAC[] = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };

void _DHCP_initPacket(NetworkInterface* ni, uint8_t op, uint32_t transactionId);
void _DHCP_handleDhcpResponse(NetworkInterface* ni, DHCP_Header* dhcpHeader);
void _DHCP_sendRequest(NetworkInterface* ni);

void _DHCP_initPacket(NetworkInterface* ni, uint8_t op, uint32_t transactionId) {
  UDP_initHeader(ni, DHCP_UDP_SOURCE_PORT, DHCP_DEST_MAC, DHCP_DEST_IP, DHCP_UDP_DEST_PORT);

  DHCP_Header* message = (DHCP_Header*)ni->bufferEnd;
  message->op = op;
  message->hardwareType = DHCP_HTYPE_ETHERNET;
  message->hardwareAddrLen = MAC_ADDRESS_SIZE;
  message->hops = 0;
  message->transactionId = htonl(transactionId);
  message->secondsElapsed = 0;
  message->flags = htons(DHCP_FLAGS_UNICAST);
  message->clientIpAddr = IPTOINT32(0, 0, 0, 0);
  message->yourIpAddr = IPTOINT32(0, 0, 0, 0);
  message->serverIpAddr = htonl(ni->dhcpData.serverIp);
  message->gatewayIpAddr = IPTOINT32(0, 0, 0, 0);
  memset(message->clientHardwareAddress, 0, sizeof(message->clientHardwareAddress));
  memcpy(message->clientHardwareAddress, ni->macAddress, MAC_ADDRESS_SIZE);
  memset(message->overflowSpace, 0, sizeof(message->overflowSpace));
  message->magicCookie = htonl(DHCP_MAGIC_COOKIE);
  ni->bufferEnd += sizeof(DHCP_Header);
}

void DHCP_sendDiscover(NetworkInterface* ni) {
  ni->dhcpData.transactionId++;
  ni->dhcpData.dhcpServer = 0;
  ni->dhcpData.serverIp = 0;
  ni->ipAddress = 0;
  _DHCP_initPacket(ni, DHCP_OP_REQUEST, ni->dhcpData.transactionId);
  uint8_t* p = ni->bufferEnd;
  uint8_t* start = p - sizeof(DHCP_Header);

  // Discover
  *p++ = DHCP_OPTION_MESSAGE_TYPE;
  *p++ = 1;
  *p++ = DHCP_MESSAGE_TYPE_DISCOVER;

  // Parameter request list
  *p++ = DHCP_OPTION_PARAM_REQ_LIST;
  *p++ = 4;
  *p++ = DHCP_PARAMETER_SUBNET_MASK;
  *p++ = DHCP_PARAMETER_BROADCAST_ADDRESS;
  *p++ = DHCP_PARAMETER_ROUTER;
  *p++ = DHCP_PARAMETER_DOMAIN_NAME_SERVER;

  // End
  *p++ = DHCP_OPTION_END;

  ni->bufferEnd = p;
  UDP_finalize(ni, p - start);
  NetworkInterface_sendBuffer(ni);
}

void _DHCP_sendRequest(NetworkInterface* ni) {
  DHCP_DEBUG_OUT("_DHCP_sendRequest\n");
  _DHCP_initPacket(ni, DHCP_OP_REQUEST, ni->dhcpData.transactionId);
  uint8_t* p = ni->bufferEnd;
  uint8_t* start = p - sizeof(DHCP_Header);

  // Request
  *p++ = DHCP_OPTION_MESSAGE_TYPE;
  *p++ = 1;
  *p++ = DHCP_MESSAGE_TYPE_REQUEST;

  // ip address
  *p++ = DHCP_OPTION_IP_REQUESTED;
  *p++ = 4;
  *((uint32_t*)p) = htonl(ni->dhcpData.proposedIp);
  p += 4;

  // dhcp server
  *p++ = DHCP_OPTION_SERVER_ID;
  *p++ = 4;
  *((uint32_t*)p) = htonl(ni->dhcpData.dhcpServer);
  p += 4;

  // hostname
  if (ni->hostName) {
    uint8_t hostNameLen = strlen(ni->hostName);
    *p++ = DHCP_OPTION_HOST_NAME;
    *p++ = hostNameLen;
    memcpy(p, ni->hostName, hostNameLen);
    p += hostNameLen;
  }

  // End
  *p++ = DHCP_OPTION_END;

  ni->bufferEnd = p;
  UDP_finalize(ni, p - start);
  NetworkInterface_sendBuffer(ni);
}

void DHCP_handleDhcpResponse(NetworkInterface* ni, DHCP_Header* dhcpHeader) {
  DHCP_DEBUG_OUT("dhcp op: %d\n", dhcpHeader->op);
  switch (dhcpHeader->op) {
  case DHCP_OP_RESPONSE:
    _DHCP_handleDhcpResponse(ni, dhcpHeader);
    break;
  }
}

void _DHCP_handleDhcpResponse(NetworkInterface* ni, DHCP_Header* dhcpHeader) {
  int i;
  uint8_t messageType = 0x00;
  ni->dhcpData.serverIp = ntohl(dhcpHeader->serverIpAddr);
  uint32_t ipAddress = ntohl(dhcpHeader->yourIpAddr);
  uint8_t* p = ((uint8_t*)dhcpHeader) + sizeof(DHCP_Header);
  while (p < ni->bufferEnd) {
    uint8_t option = *p++;
    if (option == DHCP_OPTION_END) {
      break;
    }
    uint8_t len = *p++;
    switch (option) {
    case DHCP_OPTION_MESSAGE_TYPE:
      messageType = *p;
      DHCP_DEBUG_OUT("messageType: 0x%02x\n", messageType);
      if (messageType == DHCP_MESSAGE_TYPE_NAK) {
        ni->dhcpData.dhcpServer = 0;
        ni->subnetMask = 0;
        ni->router = 0;
        for (i = 0; i < NETWORK_INTERFACE_MAX_DNS_COUNT; i++) {
          ni->dnsServers[i] = 0;
        }
      }
      break;

    case DHCP_OPTION_LEASE_TIME:
      break;

    case DHCP_OPTION_SERVER_ID:
      ni->dhcpData.dhcpServer = ntohl(*((uint32_t*)p));
      break;

    case DHCP_OPTION_SUBNET_MASK:
      if (messageType == DHCP_MESSAGE_TYPE_ACK) {
        ni->subnetMask = ntohl(*((uint32_t*)p));
        DHCP_DEBUG_OUT("subnetMask: 0x%08lx\n", ni->subnetMask);
      }
      break;

    case DHCP_OPTION_ROUTER:
      if (messageType == DHCP_MESSAGE_TYPE_ACK) {
        ni->router = ntohl(*((uint32_t*)p));
        DHCP_DEBUG_OUT("router: 0x%08lx\n", ni->router);
      }
      break;

    case DHCP_OPTION_DNS_SERVERS:
      if (messageType == DHCP_MESSAGE_TYPE_ACK) {
        for (i = 0; i < len / 4 && i < NETWORK_INTERFACE_MAX_DNS_COUNT; i++) {
          ni->dnsServers[i] = ntohl(*((uint32_t*)(p + (i * 4))));
          DHCP_DEBUG_OUT("dnsServers[%d]: 0x%08lx\n", i, ni->dnsServers[i]);
        }
        for (; i < NETWORK_INTERFACE_MAX_DNS_COUNT; i++) {
          ni->dnsServers[i] = 0;
        }
      }
      break;

    default:
      DHCP_DEBUG_OUT("unhandled dhcp option: 0x%02x\n", option);
      break;
    }
    p += len;
  }

  if (messageType == DHCP_MESSAGE_TYPE_OFFER) {
    ni->dhcpData.proposedIp = ipAddress;
    _DHCP_sendRequest(ni);
  } else if (messageType == DHCP_MESSAGE_TYPE_ACK) {
    ni->ipAddress = ipAddress;
    DHCP_DEBUG_OUT("ipAddress: 0x%08lx\n", ni->ipAddress);
  }
}
