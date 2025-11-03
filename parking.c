/* parking.c - Parking spaces with sequential lighting */
#include "contiki.h"
#include "dev/leds.h"
#include "sys/etimer.h"
#include "net/linkaddr.h"  // ADD THIS LINE
#include <stdio.h>

static int parking_id = 0;

PROCESS(parking_process, "Parking Space");
AUTOSTART_PROCESSES(&parking_process);

PROCESS_THREAD(parking_process, ev, data)
{
  static struct etimer timer;
  static int light_state = 0;
  
  PROCESS_BEGIN();

  // Get parking space ID - FIXED VERSION
  parking_id = (linkaddr_node_addr.u8[0] + linkaddr_node_addr.u8[1]) % 10;
  
  printf("Parking Space %d started - Sequential mode\n", parking_id);
  
  etimer_set(&timer, 2 * CLOCK_SECOND); // Slower pattern

  while(1) {
    PROCESS_WAIT_EVENT_UNTIL(ev == PROCESS_EVENT_TIMER);
    
    leds_off(LEDS_ALL);
    
    // Different patterns using only RED, GREEN, YELLOW
    switch(light_state % 3) {
      case 0:
        // Pattern 1: Fill parking spaces sequentially
        if(parking_id == (light_state / 3) % 10) {
          leds_on(LEDS_RED);
          printf("Parking %d: OCCUPIED (pattern 1)\n", parking_id);
        }
        break;
        
      case 1:
        // Pattern 2: Odd/even parking spaces
        if((light_state / 3) % 2 == 0) {
          if(parking_id % 2 == 0) { // Even spaces
            leds_on(LEDS_YELLOW);
            printf("Parking %d: RESERVED (even)\n", parking_id);
          }
        } else {
          if(parking_id % 2 == 1) { // Odd spaces
            leds_on(LEDS_YELLOW);
            printf("Parking %d: RESERVED (odd)\n", parking_id);
          }
        }
        break;
        
      case 2:
        // Pattern 3: All parking spaces light up in groups
        if(parking_id >= (light_state / 3) % 3 * 3 && 
           parking_id < ((light_state / 3) % 3 + 1) * 3) {
          leds_on(LEDS_GREEN);
          printf("Parking %d: AVAILABLE (group)\n", parking_id);
        }
        break;
    }
    
    light_state++;
    etimer_reset(&timer);
  }

  PROCESS_END();
}
