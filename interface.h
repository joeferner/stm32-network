
#ifndef _interface_h_
#define _interface_h_

#include <stdint.h>
#include <stdbool.h>

#ifndef MAC_ADDRESS_SIZE
#  define MAC_ADDRESS_SIZE 6
#endif
#define NETWORK_INTERFACE_BUFFER_SIZE 1024
#define NETWORK_INTERFACE_MAX_DNS_COUNT 4

#ifndef ARP_TABLE_SIZE
#  define ARP_TABLE_SIZE 10
#endif

typedef struct _NetworkInterface NetworkInterface;

typedef uint32_t NetworkInterface_read(NetworkInterface* nic, uint8_t* buffer, uint32_t len);
typedef uint32_t NetworkInterface_send(NetworkInterface* nic, uint8_t* buffer, uint32_t len);

#ifndef htons
#  define htons(n) (((((unsigned short)(n) & 0xFF)) << 8) | (((unsigned short)(n) & 0xFF00) >> 8))
#endif

#ifndef ntohs
#  define ntohs(n) (((((unsigned short)(n) & 0xFF)) << 8) | (((unsigned short)(n) & 0xFF00) >> 8))
#endif

#ifndef htonl
#  define htonl(n) (((((unsigned long)(n) & 0xFF)) << 24) | \
                  ((((unsigned long)(n) & 0xFF00)) << 8) | \
                  ((((unsigned long)(n) & 0xFF0000)) >> 8) | \
                  ((((unsigned long)(n) & 0xFF000000)) >> 24))
#endif

#ifndef ntohl
#  define ntohl(n) (((((unsigned long)(n) & 0xFF)) << 24) | \
                  ((((unsigned long)(n) & 0xFF00)) << 8) | \
                  ((((unsigned long)(n) & 0xFF0000)) >> 8) | \
                  ((((unsigned long)(n) & 0xFF000000)) >> 24))
#endif

typedef struct {
  bool valid;
  uint32_t createTime;
  uint32_t ipAddress;
  uint8_t macAddress[MAC_ADDRESS_SIZE];
} NetworkInterfaceArpTableEntry;

typedef struct {
  uint32_t transactionId;
  uint32_t serverIp;
  uint32_t dhcpServer;
  uint32_t proposedIp;
} NetworkInterfaceDhcpData;

struct _NetworkInterface {
  uint8_t macAddress[MAC_ADDRESS_SIZE];
  uint32_t ipAddress;
  uint32_t subnetMask;
  uint32_t router;
  uint32_t dnsServers[NETWORK_INTERFACE_MAX_DNS_COUNT];
  const char* hostName;
  NetworkInterface_read* receive;
  NetworkInterface_send* send;
  uint8_t buffer[NETWORK_INTERFACE_BUFFER_SIZE];
  uint8_t* bufferEnd;
  NetworkInterfaceDhcpData dhcpData;
  NetworkInterfaceArpTableEntry arpTable[ARP_TABLE_SIZE];
  struct _TCP_Server* tcpServers;
};

void NetworkInterface_init(
  NetworkInterface* ni,
  uint8_t* macAddress,
  NetworkInterface_read* receive,
  NetworkInterface_send* send
);
void NetworkInterface_tick(NetworkInterface* ni);

void NetworkInterface_sendBuffer(NetworkInterface* ni);
void NetworkInterface_addArpTableEntry(NetworkInterface* ni, uint8_t* macAddress, uint32_t ipAddress);

#endif
