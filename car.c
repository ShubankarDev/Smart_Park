/* car.c - automated or manual, similar to person */
#include "contiki.h"
#include "net/ipv6/simple-udp.h"
#include "dev/serial-line.h"
#include "sys/etimer.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define UDP_PORT 2020
#define PARKING_SLOTS 10

static struct simple_udp_connection udp_conn;
static int current_spot = 0;
static int direction = 1;
static int auto_mode = 1;

PROCESS(car_process, "Car");
AUTOSTART_PROCESSES(&car_process);

PROCESS_THREAD(car_process, ev, data)
{
  static struct etimer timer;
  PROCESS_BEGIN();

  serial_line_init();
  simple_udp_register(&udp_conn, 0, NULL, UDP_PORT, NULL);
  printf("Car started. Auto move every 4s.\n");
  printf("Commands: CAR_POS n, AUTO 0/1\n");

  etimer_set(&timer, 4 * CLOCK_SECOND);

  while(1) {
    PROCESS_WAIT_EVENT();
    if(ev == PROCESS_EVENT_TIMER && etimer_expired(&timer) && auto_mode) {
      current_spot += direction;
      if(current_spot >= PARKING_SLOTS) { 
        direction = -1; 
        current_spot = PARKING_SLOTS - 2; 
      }
      if(current_spot < 0) { 
        direction = 1; 
        current_spot = 1; 
      }

      char msg[64];
      snprintf(msg, sizeof(msg), "CAR_POS %d", current_spot);
      uip_ipaddr_t dest;
      uip_ip6addr(&dest, 0xff02,0,0,0,0,0,0,1);
      simple_udp_sendto(&udp_conn, msg, strlen(msg), &dest);
      printf("Car auto -> spot %d\n", current_spot);
      etimer_reset(&timer);
    } else if(ev == serial_line_event_message) {
      char *line = (char*)data;
      if(strncmp(line, "CAR_POS ", 8) == 0) {
        int n = atoi(line + 8);
        if(n >= 0 && n < PARKING_SLOTS) {
          current_spot = n;
          char msg[64];
          snprintf(msg, sizeof(msg), "CAR_POS %d", current_spot);
          uip_ipaddr_t dest;
          uip_ip6addr(&dest, 0xff02,0,0,0,0,0,0,1);
          simple_udp_sendto(&udp_conn, msg, strlen(msg), &dest);
          printf("Car manual -> spot %d\n", current_spot);
        } else {
          printf("Invalid CAR_POS value (0-%d)\n", PARKING_SLOTS-1);
        }
      } else if(strncmp(line, "AUTO ", 5) == 0) {
        int v = atoi(line + 5);
        auto_mode = (v != 0);
        printf("Car Auto mode -> %d\n", auto_mode);
        if(auto_mode) etimer_reset(&timer);
      } else {
        printf("Unknown command: %s\n", line);
      }
    }
  }

  PROCESS_END();
}
