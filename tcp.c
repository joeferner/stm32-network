
#include "tcp.h"
#include "ipv4.h"
#include "mac.h"
#include "checksum.h"
#include <stdio.h>
#include <string.h>
#include "platform_config.h"

#ifdef TCP_DEBUG
#define TCP_DEBUG_OUT(format, ...) printf("%s:%d: TCP: " format, __FILE__, __LINE__, ##__VA_ARGS__)
#else
#define TCP_DEBUG_OUT(format, ...)
#endif

void _TCP_handlePacketForServer(NetworkInterface* ni, TCP_Server* server, TCP_Header* tcpHeader);
void _TCP_handlePacketForServerConnection(NetworkInterface* ni, TCP_Server* server, TCP_Connection* conn, TCP_Header* tcpHeader);
void _TCP_replySynAck(NetworkInterface* ni, TCP_Connection* conn);
void _TCP_handleSynAckResponse(NetworkInterface* ni, TCP_Connection* conn);
void _TCP_handleDataPacket(NetworkInterface* ni, TCP_Connection* conn);
void _TCP_sendAck(NetworkInterface* ni, TCP_Connection* conn);
void _TCP_sendFinAck(NetworkInterface* ni, TCP_Connection* conn);
void _TCP_handleFin(NetworkInterface* ni, TCP_Connection* conn);
void _TCP_initReplyHeader(NetworkInterface* ni, TCP_Connection* connection, uint16_t flags);

void TCP_listen(
  NetworkInterface* ni,
  TCP_Server* server,
  uint16_t port,
  TCP_Connection* connections,
  int connectionsCount,
  tcp_connect* connect,
  tcp_disconnect* disconnect,
  tcp_receive* receive
) {
  TCP_DEBUG_OUT("TCP_listen(port: %d)\n", port);
  memset(server, 0, sizeof(TCP_Server));
  memset(connections, 0, sizeof(TCP_Connection) * connectionsCount);
  for (int i = 0; i < connectionsCount; i++) {
    connections[i].server = server;
  }

  if (ni->tcpServers) {
    TCP_Server* lastServer = ni->tcpServers;
    while (lastServer->next) {
      lastServer = lastServer->next;
    }
    lastServer->next = server;
  } else {
    ni->tcpServers = server;
  }

  server->port = port;
  server->connections = connections;
  server->connectionsCount = connectionsCount;
  server->connect = connect;
  server->disconnect = disconnect;
  server->receive = receive;
}

TCP_Header* TCP_getHeader(NetworkInterface* ni) {
  return (TCP_Header*)(ni->buffer + sizeof(MAC_Header) + sizeof(IPV4_Header));
}

uint8_t* TCP_getData(NetworkInterface* ni) {
  TCP_Header* header = TCP_getHeader(ni);
  uint8_t* p = (uint8_t*)header;
  return p + TCP_HEADER_LENGTH(header);
}

void TCP_initHeader(NetworkInterface* ni, TCP_Connection* connection, uint16_t flags) {
  IPV4_initHeader(ni, IPV4_Protocol_TCP, connection->remoteMac, connection->remoteIpAddress);
  TCP_Header* tcpHeader = TCP_getHeader(ni);
  tcpHeader->sourcePort = htons(connection->localPort);
  tcpHeader->destPort = htons(connection->remotePort);
  tcpHeader->sequenceNumber = htonl(connection->sequenceNumber);
  tcpHeader->ackNumber = htonl(connection->previousAckNumber);
  tcpHeader->headerLengthAndFlags = TCP_HEADER_LENGTH_AND_FLAGS(sizeof(TCP_Header), flags);
  tcpHeader->windowSize = htons(TCP_WINDOW_SIZE);
  tcpHeader->checksum = htons(0x0000);
  tcpHeader->urgetPointer = htons(0x0000);
  ni->bufferEnd += TCP_HEADER_LENGTH(tcpHeader);
}

void _TCP_initReplyHeader(NetworkInterface* ni, TCP_Connection* connection, uint16_t flags) {
  IPV4_Header* ipHeader = IPV4_getHeader(ni);
  TCP_Header* header = TCP_getHeader(ni);
  uint32_t previousDataSize = ntohs(ipHeader->totalLength)
                              - sizeof(IPV4_Header)
                              - TCP_HEADER_LENGTH(header);
  int synFinAckAdd1 = (TCP_HEADER_TEST_FLAG(header, TCP_HEADER_FLAGS_SYN) || TCP_HEADER_TEST_FLAG(header, TCP_HEADER_FLAGS_FIN)) ? 1 : 0;
  IPV4_initReplyHeader(ni);
  TCP_Header* tcpHeader = TCP_getHeader(ni);

  uint16_t origSourcePort = tcpHeader->sourcePort;
  uint16_t origDestPort = tcpHeader->destPort;
  uint32_t origSequenceNumber = ntohl(tcpHeader->sequenceNumber);

  tcpHeader->sourcePort = origDestPort;
  tcpHeader->destPort = origSourcePort;
  tcpHeader->sequenceNumber = htonl(connection->sequenceNumber);
  tcpHeader->headerLengthAndFlags = TCP_HEADER_LENGTH_AND_FLAGS(sizeof(TCP_Header), flags);
  tcpHeader->windowSize = htons(TCP_WINDOW_SIZE);
  tcpHeader->ackNumber = htonl(origSequenceNumber + previousDataSize + synFinAckAdd1);
  connection->sequenceNumber += synFinAckAdd1;

  ni->bufferEnd += TCP_HEADER_LENGTH(tcpHeader);
}

void TCP_finalize(NetworkInterface* ni, TCP_Connection* connection, uint16_t dataLength) {
  IPV4_finalize(ni, sizeof(TCP_Header) + dataLength);

  IPV4_Header* ipHeader = IPV4_getHeader(ni);
  TCP_Header* tcpHeader = TCP_getHeader(ni);
  TCP_ChecksumPseudoHeader pseudoHeader;
  pseudoHeader.sourceAddr = ipHeader->sourceIpAddress;
  pseudoHeader.destAddr = ipHeader->destIpAddress;
  pseudoHeader.reserved = 0;
  pseudoHeader.protocol = IPV4_Protocol_TCP;
  pseudoHeader.length = htons(TCP_HEADER_LENGTH(tcpHeader) + dataLength);

  tcpHeader->headerLengthAndFlags = TCP_HEADER_LENGTH_AND_FLAGS(sizeof(TCP_Header), TCP_HEADER_FLAGS(tcpHeader));
  tcpHeader->checksum = 0x0000;
  tcpHeader->checksum = htons(
                          Checksum_calculateOnesComplement2(
                            (uint8_t*)&pseudoHeader, sizeof(TCP_ChecksumPseudoHeader),
                            (uint8_t*)tcpHeader, TCP_HEADER_LENGTH(tcpHeader) + dataLength
                          )
                        );

  connection->sequenceNumber += dataLength;
  connection->previousAckNumber = ntohl(tcpHeader->ackNumber);
  ni->bufferEnd = TCP_getData(ni) + dataLength;
}

void TCP_handlePacket(NetworkInterface* ni, TCP_Header* tcpHeader) {
  TCP_DEBUG_OUT("TCP_handlePacket(dstport: %d)\n", ntohs(tcpHeader->destPort));
  TCP_Server* server = ni->tcpServers;
  while (server) {
    if (server->port == ntohs(tcpHeader->destPort)) {
      _TCP_handlePacketForServer(ni, server, tcpHeader);
      return;
    }
    server = server->next;
  }
}

void _TCP_handlePacketForServer(NetworkInterface* ni, TCP_Server* server, TCP_Header* tcpHeader) {
  int i;
  for (i = 0; i < server->connectionsCount; i++) {
    TCP_Connection* conn = &server->connections[i];
    if (conn->state == TCP_State_Disconnected) {
      break;
    }

    if (conn->remotePort == ntohs(tcpHeader->sourcePort)) {
      _TCP_handlePacketForServerConnection(ni, server, conn, tcpHeader);
      return;
    }
  }

  if (i < server->connectionsCount) {
    TCP_DEBUG_OUT("new connection\n");
    MAC_Header* macHeader = MAC_getHeader(ni);
    IPV4_Header* ipHeader = IPV4_getHeader(ni);
    TCP_Connection* conn = &server->connections[i];
    conn->localPort = ntohs(tcpHeader->destPort);
    conn->remotePort = ntohs(tcpHeader->sourcePort);
    conn->remoteIpAddress = ntohl(ipHeader->sourceIpAddress);
    memcpy(conn->remoteMac, macHeader->sourceMac, MAC_ADDRESS_SIZE);
    _TCP_handlePacketForServerConnection(ni, server, conn, tcpHeader);
  }
}

void _TCP_handlePacketForServerConnection(NetworkInterface* ni, TCP_Server* server, TCP_Connection* conn, TCP_Header* tcpHeader) {
  TCP_DEBUG_OUT("_TCP_handlePacketForServerConnection %d\n", conn->state);
  switch (conn->state) {
  case TCP_State_Disconnected:
    if (conn->server->connect) {
      conn->server->connect(ni, conn);
    }
    _TCP_replySynAck(ni, conn);
    break;
  case TCP_State_WaitingForSynAckResponse:
    _TCP_handleSynAckResponse(ni, conn);
    break;
  case TCP_State_Connected:
    _TCP_handleDataPacket(ni, conn);
    break;
  case TCP_State_Disconnecting: // waiting for fin ack
    conn->state = TCP_State_Disconnected;
    if (conn->server->disconnect) {
      conn->server->disconnect(ni, conn);
    }
    break;
  default:
    TCP_DEBUG_OUT("unhandled state %d\n", conn->state);
    break;
  }
}

void _TCP_handleSynAckResponse(NetworkInterface* ni, TCP_Connection* conn) {
  TCP_Header* header = TCP_getHeader(ni);
  if (TCP_HEADER_TEST_FLAG(header, TCP_HEADER_FLAGS_ACK)) {
    TCP_DEBUG_OUT("connected\n");
    conn->state = TCP_State_Connected;
  } else {
    TCP_DEBUG_OUT("unhandled flags 0x%02x\n", TCP_HEADER_FLAGS(header));
  }
}

void _TCP_replySynAck(NetworkInterface* ni, TCP_Connection* conn) {
  _TCP_initReplyHeader(ni, conn, TCP_HEADER_FLAGS_SYN | TCP_HEADER_FLAGS_ACK);
  TCP_finalize(ni, conn, 0);
  NetworkInterface_sendBuffer(ni);
  conn->state = TCP_State_WaitingForSynAckResponse;
}

void _TCP_sendAck(NetworkInterface* ni, TCP_Connection* conn) {
  _TCP_initReplyHeader(ni, conn, TCP_HEADER_FLAGS_ACK);
  TCP_finalize(ni, conn, 0);
  NetworkInterface_sendBuffer(ni);
}

void _TCP_sendFinAck(NetworkInterface* ni, TCP_Connection* conn) {
  _TCP_initReplyHeader(ni, conn, TCP_HEADER_FLAGS_FIN | TCP_HEADER_FLAGS_ACK);
  TCP_finalize(ni, conn, 0);
  NetworkInterface_sendBuffer(ni);
}

void _TCP_handleFin(NetworkInterface* ni, TCP_Connection* conn) {
  conn->state = TCP_State_Disconnecting;
  _TCP_sendFinAck(ni, conn);
}

void _TCP_handleDataPacket(NetworkInterface* ni, TCP_Connection* conn) {
  IPV4_Header* ipHeader = IPV4_getHeader(ni);
  TCP_Header* header = TCP_getHeader(ni);

  if (TCP_HEADER_TEST_FLAG(header, TCP_HEADER_FLAGS_FIN)) {
    _TCP_handleFin(ni, conn);
    return;
  }

  uint32_t dataSize = ntohs(ipHeader->totalLength) - sizeof(IPV4_Header) - TCP_HEADER_LENGTH(header);
  uint8_t* data = ((uint8_t*)header) + TCP_HEADER_LENGTH(header);

  _TCP_sendAck(ni, conn);

  TCP_DEBUG_OUT("data packet (size: %ld)\n", dataSize);
  if (conn->server->receive) {
    conn->server->receive(ni, conn, data, dataSize);
  }
}

void TCP_send(NetworkInterface* ni, TCP_Connection* conn, uint8_t* data, uint32_t dataLength) {
  TCP_DEBUG_OUT("TCP_send(length: %ld)\n", dataLength);
  TCP_initHeader(ni, conn, TCP_HEADER_FLAGS_PSH | TCP_HEADER_FLAGS_ACK);
  uint8_t* p = TCP_getData(ni);
  memcpy(p, data, dataLength);
  TCP_finalize(ni, conn, dataLength);
  NetworkInterface_sendBuffer(ni);
}

bool TCP_isConnected(TCP_Connection* connection) {
  return connection->state == TCP_State_Connected;
}
