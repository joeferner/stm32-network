
#ifndef _tcp_h_
#define _tcp_h_

#include <stdint.h>
#include "interface.h"

#ifndef TCP_WINDOW_SIZE
#define TCP_WINDOW_SIZE (NETWORK_INTERFACE_BUFFER_SIZE - sizeof(MAC_Header) - sizeof(IPV4_Header) - sizeof(TCP_Header))
#endif

#define TCP_HEADER_FLAGS_FIN 0b000000001
#define TCP_HEADER_FLAGS_SYN 0b000000010
#define TCP_HEADER_FLAGS_RST 0b000000100
#define TCP_HEADER_FLAGS_PSH 0b000001000
#define TCP_HEADER_FLAGS_ACK 0b000010000
#define TCP_HEADER_FLAGS_URG 0b000100000
#define TCP_HEADER_FLAGS_ECE 0b001000000
#define TCP_HEADER_FLAGS_CWR 0b010000000
#define TCP_HEADER_FLAGS_NS  0b100000000

#define TCP_HEADER_LENGTH_AND_FLAGS(headerLength, flags) htons((((headerLength) / 4) << 12) | flags)
#define TCP_HEADER_FLAGS(tcpHeader) (ntohs(tcpHeader->headerLengthAndFlags) & 0x0fff)
#define TCP_HEADER_TEST_FLAG(tcpHeader, flag) ((TCP_HEADER_FLAGS(tcpHeader) & flag) == flag)
#define TCP_HEADER_LENGTH(tcpHeader) (4 * (ntohs(tcpHeader->headerLengthAndFlags) >> 12))

typedef struct _TCP_Connection TCP_Connection;
typedef struct _TCP_Server TCP_Server;

typedef void(tcp_connect)(NetworkInterface* nic, TCP_Connection* conn);
typedef void(tcp_disconnect)(NetworkInterface* nic, TCP_Connection* conn);
typedef void(tcp_receive)(NetworkInterface* nic, TCP_Connection* conn, uint8_t* data, uint32_t dataLength);

typedef enum {
  TCP_State_Disconnected = 0,
  TCP_State_WaitingForSynAckResponse = 1,
  TCP_State_Connected = 2,
  TCP_State_Disconnecting = 3
} TCP_State;

#pragma pack(push,1)
typedef struct {
  uint32_t sourceAddr;
  uint32_t destAddr;
  uint8_t reserved;
  uint8_t protocol;
  uint16_t length;
} TCP_ChecksumPseudoHeader;

typedef struct {
  uint16_t sourcePort;
  uint16_t destPort;
  uint32_t sequenceNumber;
  uint32_t ackNumber;
  uint16_t headerLengthAndFlags;
  uint16_t windowSize;
  uint16_t checksum;
  uint16_t urgetPointer;
} TCP_Header;
#pragma pack(pop)

typedef struct _TCP_Connection {
  TCP_State state;
  uint16_t localPort;
  uint16_t remotePort;
  uint8_t remoteMac[MAC_ADDRESS_SIZE];
  uint32_t remoteIpAddress;
  uint32_t sequenceNumber;
  uint32_t previousAckNumber;
  TCP_Server* server;
} TCP_Connection;

typedef struct _TCP_Server {
  uint16_t port;
  TCP_Connection* connections;
  int connectionsCount;
  TCP_Server* next;
  tcp_connect* connect;
  tcp_disconnect* disconnect;
  tcp_receive* receive;
} TCP_Server;

void TCP_listen(
  NetworkInterface* ni,
  TCP_Server* server,
  uint16_t port,
  TCP_Connection* connections,
  int connectionsCount,
  tcp_connect* connect,
  tcp_disconnect* disconnect,
  tcp_receive* receive
);

void TCP_handlePacket(NetworkInterface* ni, TCP_Header* tcpHeader);
void TCP_initHeader(NetworkInterface* ni, TCP_Connection* connection, uint16_t flags);
void TCP_finalize(NetworkInterface* ni, TCP_Connection* connection, uint16_t dataLength);
void TCP_send(NetworkInterface* ni, TCP_Connection* connection, uint8_t* data, uint32_t dataLength);
TCP_Header* TCP_getHeader(NetworkInterface* ni);
uint8_t* TCP_getData(NetworkInterface* ni);
bool TCP_isConnected(TCP_Connection* connection);

#endif
