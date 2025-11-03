/* moisture.c - soil moisture simulation; blue LED indicates dry */
#include "contiki.h"
#include "net/ipv6/simple-udp.h"
#include "dev/leds.h"
#include "sys/etimer.h"
#include <stdio.h>
#include <stdlib.h>

#define UDP_PORT 2020
static struct simple_udp_connection udp_conn;
static unsigned int seed = 0;

PROCESS(moisture_process, "Soil Moisture");
AUTOSTART_PROCESSES(&moisture_process);

PROCESS_THREAD(moisture_process, ev, data)
{
  static struct etimer timer;
  PROCESS_BEGIN();

  simple_udp_register(&udp_conn, UDP_PORT, NULL, UDP_PORT, NULL);
  etimer_set(&timer, 6 * CLOCK_SECOND);
  
  // Simple seed based on node address
  seed = linkaddr_node_addr.u8[0] + linkaddr_node_addr.u8[1] + 7;
  
  printf("Soil moisture node started\n");

  while(1) {
    PROCESS_WAIT_EVENT_UNTIL(ev == PROCESS_EVENT_TIMER);
    
    // Simple pseudo-random for Cooja
    seed = (seed * 1103515245 + 12345) & 0x7FFFFFFF;
    int moisture = seed % 101; /* 0..100 */
    
    char buf[64];
    snprintf(buf, sizeof(buf), "MOISTURE %d", moisture);
    uip_ipaddr_t dest;
    uip_ip6addr(&dest, 0xff02,0,0,0,0,0,0,1);
    simple_udp_sendto(&udp_conn, buf, strlen(buf), &dest);
    printf("[Soil] %d%%\n", moisture);

    if(moisture < 30) {
      leds_on(LEDS_BLUE);
    } else {
      leds_off(LEDS_BLUE);
    }

    etimer_reset(&timer);
  }

  PROCESS_END();
}
