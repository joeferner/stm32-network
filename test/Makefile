.PHONY: all test

CC         = gcc
CFLAGS     = -c -Wall -I. -I.. -I../.. -DNETWORK_INTERFACE_DEBUG -DNETWORK_INTERFACE_DEBUG_PACKETS -DTCP_DEBUG
LDFLAGS    =
SOURCES    = \
  platform.c \
  test.c \
  test-mac.c \
  test-dhcp.c \
  test-ipv4.c \
  test-udp.c \
  test-icmp.c \
  test-tcp.c \
  test-checksum.c \
  ../arp.c \
  ../mac.c \
  ../dhcp.c \
  ../interface.c \
  ../ipv4.c \
  ../checksum.c \
  ../icmp.c \
  ../udp.c \
  ../tcp.c
OBJECTS    = $(addprefix build/test/,$(SOURCES:.c=.o))
EXECUTABLE = build/network-test

all: $(SOURCES) $(EXECUTABLE)

clean:
	rm -rf ./build

test: all
	./build/network-test

$(EXECUTABLE): $(OBJECTS) 
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@

build/test/%.o: %.c
	@mkdir -p build/test
	$(CC) $(CFLAGS) $< -o $@
