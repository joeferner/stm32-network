
#include "test.h"
#include "../ipv4.h"
#include "../tcp.h"
#include "../mac.h"
#include "../checksum.h"
#include <string.h>

extern NetworkInterface ni;
extern uint8_t readBuffer[5000];
extern uint32_t readBufferLength;
extern uint8_t sendBuffer[5000];
extern uint32_t sendBufferLength;

static char* tcp_test_checksum();
static char* tcp_test_server();

char* tcp_test() {
  mu_run_test(tcp_test_checksum);
  mu_run_test(tcp_test_server);
  return NULL;
}

static char* tcp_test_checksum() {
  uint8_t tcpPacket[] = {
    0x00, 0x17, 0xc0, 0xa4, 0x00, 0x00, 0x00, 0x00, 0x14, 0xbe, 0x4f, 0xb8, 0xa0, 0x12, 0x72, 0x10,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
  };

  TCP_ChecksumPseudoHeader pseudoHeader;
  pseudoHeader.sourceAddr = htonl(0xc0a80164);
  pseudoHeader.destAddr = htonl(0xc0a80165);
  pseudoHeader.reserved = 0;
  pseudoHeader.protocol = IPV4_Protocol_TCP;
  pseudoHeader.length = htons(sizeof(TCP_Header) + 20);
    
  mu_assert_equals_int(sizeof(TCP_Header) + 20, sizeof(tcpPacket));
  
  uint16_t checksum = Checksum_calculateOnesComplement2(
    (uint8_t*)&pseudoHeader, sizeof(TCP_ChecksumPseudoHeader),
    (uint8_t*)tcpPacket, sizeof(TCP_Header) + 20
  );
  mu_assert_equals_int(0x4462, checksum);
  return NULL;
}

static char* tcp_test_server() {
  TCP_Server server;
  TCP_Connection serverConnections[10];
  int serverPort = 80;
  
  TCP_listen(&ni, &server, serverPort, serverConnections, 10, NULL, NULL, NULL);
  
  uint8_t clientSynPacket[] = {
    0x01, 0x02, 0x03, 0x04, 0x05, 0x06, // dest mac address
    0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, // source mac address
    0x08, 0x00, // type ipv4

    // ipv4
    0x45, // version and header length
    0x00,
    0x00, 0x3c, // total length
    0x00, 0x00, // identification
    0x40, 0x00, // flags, offset
    0xff, // ttl
    0x06, // protocol - tcp 
    0x00, 0x00, // checksum
    0xc0, 0xa8, 0x00, 0x01, // source ip - 192.168.0.1
    0xc0, 0xa8, 0x00, 0x02, // dest ip - 192.168.0.2
    
    // tcp
    0x01, 0x02, // source port
    0x00, 0x50, // dest port - 80
    0x00, 0x00, 0x00, 0x01, // sequence number
    0x00, 0x00, 0x00, 0x00, // ack sequence number
    0xa0, 0x02, // header length, flags (SYN)
    0xff, 0xff, // window size
    0x00, 0x00, // checksum
    0x00, 0x00, // urgent
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 // options
  };
  memcpy(readBuffer, clientSynPacket, sizeof(clientSynPacket));
  readBufferLength = sizeof(clientSynPacket);
  NetworkInterface_tick(&ni);
  
  uint8_t expectedSynAckBytes[] = {
    0x00, 0x50, // source port - 80
    0x01, 0x02, // dest port
    0x00, 0x00, 0x00, 0x00, // sequence number
    0x00, 0x00, 0x00, 0x02, // ack sequence number
    0x50, 0x12, // header length, flags (SYN, ACK)
    0x03, 0xca, // window size
    0xea, 0x0b, // checksum
    0x00, 0x00 // urgent
  };
  int tcpPacketOffset = sizeof(MAC_Header) + sizeof(IPV4_Header);
  mu_assert_equals_byteArray(
    expectedSynAckBytes,
    sizeof(expectedSynAckBytes),
    ni.buffer + tcpPacketOffset,
    ni.bufferEnd - ni.buffer - tcpPacketOffset
  );

  uint8_t clientAckPacket[] = {
    0x01, 0x02, 0x03, 0x04, 0x05, 0x06, // dest mac address
    0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, // source mac address
    0x08, 0x00, // type ipv4

    // ipv4
    0x45, // version and header length
    0x00,
    0x00, 0x3c, // total length
    0x00, 0x00, // identification
    0x40, 0x00, // flags, offset
    0xff, // ttl
    0x06, // protocol - tcp 
    0x00, 0x00, // checksum
    0xc0, 0xa8, 0x00, 0x01, // source ip - 192.168.0.1
    0xc0, 0xa8, 0x00, 0x02, // dest ip - 192.168.0.2
    
    // tcp
    0x01, 0x02, // source port
    0x00, 0x50, // dest port - 80
    0x00, 0x00, 0x00, 0x01, // sequence number
    0x00, 0x00, 0x00, 0x00, // ack sequence number
    0xa0, 0x02, // header length, flags (SYN)
    0xff, 0xff, // window size
    0x00, 0x00, // checksum
    0x00, 0x00, // urgent
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 // options
  };
  memcpy(readBuffer, clientAckPacket, sizeof(clientAckPacket));
  readBufferLength = sizeof(clientAckPacket);
  NetworkInterface_tick(&ni);
  
  return NULL;
}
