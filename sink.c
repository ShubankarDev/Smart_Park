/* sink.c - central controller */
#include "contiki.h"
#include "net/ipv6/simple-udp.h"
#include "dev/serial-line.h"
#include "sys/etimer.h"
#include <stdio.h>
#include <string.h>

#define UDP_PORT 2020
static struct simple_udp_connection udp_conn;

static void print_ts(void) {
  static unsigned long t = 0;
  t++;
  printf("[%06lu] ", t);
}

static void
recv_from_node(struct simple_udp_connection *c,
               const uip_ipaddr_t *sender_addr, uint16_t sender_port,
               const uip_ipaddr_t *receiver_addr, uint16_t receiver_port,
               const uint8_t *data, uint16_t datalen)
{
  char buf[128];
  if(datalen >= sizeof(buf)) datalen = sizeof(buf)-1;
  memcpy(buf, data, datalen);
  buf[datalen] = '\0';

  print_ts();
  printf("Sink received: %s\n", buf);
}

PROCESS(sink_process, "Sink Node");
AUTOSTART_PROCESSES(&sink_process);

PROCESS_THREAD(sink_process, ev, data)
{
  PROCESS_BEGIN();

  simple_udp_register(&udp_conn, UDP_PORT, NULL, UDP_PORT, recv_from_node);
  serial_line_init();

  printf("=== SmartPark Sink ===\n");
  printf("Commands: ALL_GLOW, ALL_OFF, TEST_SCENARIO\n");

  while(1) {
    PROCESS_WAIT_EVENT();
    if(ev == serial_line_event_message) {
      char *line = (char*)data;
      uip_ipaddr_t dest;
      uip_ip6addr(&dest, 0xff02,0,0,0,0,0,0,1);
      
      if(strcmp(line, "TEST_SCENARIO") == 0) {
        printf("=== Starting Demo Scenario ===\n");
        simple_udp_sendto(&udp_conn, "PERSON_POS 5", 12, &dest);
        simple_udp_sendto(&udp_conn, "CAR_POS 8", 9, &dest);
        simple_udp_sendto(&udp_conn, "ALL_GLOW", 8, &dest);
      } else {
        simple_udp_sendto(&udp_conn, line, strlen(line), &dest);
        print_ts(); printf("Sink: Broadcasted '%s'\n", line);
      }
    }
  }

  PROCESS_END();
}
