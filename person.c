/* person.c - moving person: automated or manual via serial (POS n) */
#include "contiki.h"
#include "net/ipv6/simple-udp.h"
#include "dev/serial-line.h"
#include "sys/etimer.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define UDP_PORT 2020
#define TOTAL_LIGHTS 17

static struct simple_udp_connection udp_conn;
static int current_position = 0;
static int auto_mode = 1;

PROCESS(person_process, "Person");
AUTOSTART_PROCESSES(&person_process);

PROCESS_THREAD(person_process, ev, data)
{
  static struct etimer timer;
  PROCESS_BEGIN();

  serial_line_init();
  simple_udp_register(&udp_conn, 0, NULL, UDP_PORT, NULL);

  printf("Person started. Auto-walk every 3s.\n");
  printf("Commands: POS n, AUTO 0/1\n");

  etimer_set(&timer, 3 * CLOCK_SECOND);

  while(1) {
    PROCESS_WAIT_EVENT();
    if(ev == PROCESS_EVENT_TIMER && etimer_expired(&timer) && auto_mode) {
      char msg[64];
      snprintf(msg, sizeof(msg), "PERSON_POS %d", current_position);
      uip_ipaddr_t dest;
      uip_ip6addr(&dest, 0xff02,0,0,0,0,0,0,1);
      simple_udp_sendto(&udp_conn, msg, strlen(msg), &dest);
      printf("Person auto -> position %d\n", current_position);
      current_position = (current_position + 1) % TOTAL_LIGHTS;
      etimer_reset(&timer);
    } else if(ev == serial_line_event_message) {
      char *line = (char*)data;
      if(strncmp(line, "POS ", 4) == 0) {
        int n = atoi(line + 4);
        if(n >= 0 && n < TOTAL_LIGHTS) {
          current_position = n;
          char msg[64];
          snprintf(msg, sizeof(msg), "PERSON_POS %d", current_position);
          uip_ipaddr_t dest;
          uip_ip6addr(&dest, 0xff02,0,0,0,0,0,0,1);
          simple_udp_sendto(&udp_conn, msg, strlen(msg), &dest);
          printf("Person manual -> position %d\n", current_position);
        } else {
          printf("Invalid POS value (0-%d)\n", TOTAL_LIGHTS-1);
        }
      } else if(strncmp(line, "AUTO ", 5) == 0) {
        int v = atoi(line + 5);
        auto_mode = (v != 0);
        printf("Auto mode -> %d\n", auto_mode);
        if(auto_mode) etimer_reset(&timer);
      } else {
        printf("Unknown command: %s\n", line);
      }
    }
  }

  PROCESS_END();
}
