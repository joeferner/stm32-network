
#ifndef _dhcp_h_
#define _dhcp_h_

#include "ipv4.h"
#include "interface.h"

#define DHCP_UDP_SOURCE_PORT 68
#define DHCP_UDP_DEST_PORT   67
#define DHCP_DEST_IP         IPTOINT32(255, 255, 255, 255)

#define DHCP_OP_REQUEST      0x01
#define DHCP_OP_RESPONSE     0x02

#define DHCP_HTYPE_ETHERNET  0x01

#define DHCP_FLAGS_UNICAST   0x0000

#define DHCP_MAGIC_COOKIE    0x63825363

#define DHCP_OPTION_SUBNET_MASK    0x01
#define DHCP_OPTION_ROUTER         0x03
#define DHCP_OPTION_DNS_SERVERS    0x06
#define DHCP_OPTION_HOST_NAME      0x0c
#define DHCP_OPTION_IP_REQUESTED   0x32
#define DHCP_OPTION_LEASE_TIME     0x33
#define DHCP_OPTION_MESSAGE_TYPE   0x35
#define DHCP_OPTION_SERVER_ID      0x36
#define DHCP_OPTION_PARAM_REQ_LIST 0x37
#define DHCP_OPTION_END            0xff

#define DHCP_PARAMETER_SUBNET_MASK         0x01
#define DHCP_PARAMETER_TIME_OFFSET         0x02
#define DHCP_PARAMETER_ROUTER              0x03
#define DHCP_PARAMETER_TIME_SERVER         0x04
#define DHCP_PARAMETER_NAME_SERVER         0x05
#define DHCP_PARAMETER_DOMAIN_NAME_SERVER  0x06
#define DHCP_PARAMETER_LOG_SERVER          0x07
#define DHCP_PARAMETER_COOKIE_SERVER       0x08
#define DHCP_PARAMETER_LPR_SERVER          0x09
#define DHCP_PARAMETER_IMPRESS_SERVER      0x0a
#define DHCP_PARAMETER_RES_LOCATION_SERVER 0x0b
#define DHCP_PARAMETER_HOST_NAME           0x0c
#define DHCP_PARAMETER_BOOT_FILE_SIZE      0x0d
#define DHCP_PARAMETER_MERIT_DUMP_FILE     0x0e
#define DHCP_PARAMETER_DOMAIN_NAME         0x0f
#define DHCP_PARAMETER_SWAP_SERVER         0x10
#define DHCP_PARAMETER_ROOT_PATH           0x11
#define DHCP_PARAMETER_EXTENSIONS_PATH     0x12
#define DHCP_PARAMETER_BROADCAST_ADDRESS   0x1c

#define DHCP_MESSAGE_TYPE_DISCOVER 1
#define DHCP_MESSAGE_TYPE_OFFER    2
#define DHCP_MESSAGE_TYPE_REQUEST  3
#define DHCP_MESSAGE_TYPE_ACK      5
#define DHCP_MESSAGE_TYPE_NAK      6

#pragma pack(push,1)
typedef struct {
  uint8_t op;
  uint8_t hardwareType;
  uint8_t hardwareAddrLen;
  uint8_t hops;
  uint32_t transactionId;
  uint16_t secondsElapsed;
  uint16_t flags;
  uint32_t clientIpAddr;
  uint32_t yourIpAddr;
  uint32_t serverIpAddr;
  uint32_t gatewayIpAddr;
  uint8_t clientHardwareAddress[16];
  uint8_t overflowSpace[192];
  uint32_t magicCookie;
} DHCP_Header;
#pragma pack(pop)

void DHCP_sendDiscover(NetworkInterface* ni);
void DHCP_handleDhcpResponse(NetworkInterface* ni, DHCP_Header* dhcpHeader);

#endif
