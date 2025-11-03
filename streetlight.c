#include "contiki.h"
#include "dev/leds.h"
#include "sys/etimer.h"
#include "net/linkaddr.h"  // ADDED for linkaddr_node_addr
#include "sys/clock.h"     // ADDED for clock_time()
#include <stdio.h>
#include <stdint.h>        // ADDED for uint8_t

static int my_id = 0;

PROCESS(streetlight_process, "Streetlight");
AUTOSTART_PROCESSES(&streetlight_process);

PROCESS_THREAD(streetlight_process, ev, data)
{
  static struct etimer timer;
  static unsigned long start_time = 0;
  static int has_id = 0;
  
  PROCESS_BEGIN();

  // Generate unique ID from node address
  if(!has_id) {
    const uint8_t *addr = linkaddr_node_addr.u8;
    my_id = (addr[0] + addr[1]) % 17 + 1; // 1-17
    has_id = 1;
    printf("Streetlight %d ready\n", my_id);
  }
  
  // All motes start at the same time
  start_time = clock_time();
  etimer_set(&timer, 0.1 * CLOCK_SECOND);

  while(1) {
    PROCESS_WAIT_EVENT_UNTIL(ev == PROCESS_EVENT_TIMER);
    
    // Calculate current light based on elapsed time
    unsigned long elapsed_seconds = (clock_time() - start_time) / CLOCK_SECOND;
    int current_light = (elapsed_seconds / 3) % 17 + 1;
    
    // Turn off all LEDs
    leds_off(LEDS_ALL);
    
    // If this is the current light, turn on
    if(my_id == current_light) {
      leds_on(LEDS_ALL);
      
      if(my_id == 1) {
        printf("Welcome to street 207\n");
        printf("Light 1 glows\n");
      } else if(my_id == 17) {
        printf("Light 17 glows\n");
        printf("Thanks for Visiting\n");
      } else {
        printf("Light %d glows\n", my_id);
      }
    }
    
    etimer_reset(&timer);
  }

  PROCESS_END();
}
