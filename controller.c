#include "contiki.h"
#include "net/ipv6/simple-udp.h"
#include "sys/etimer.h"
#include <stdio.h>
#include <string.h>

#define UDP_PORT 2020
static struct simple_udp_connection udp_conn;
static int current_light = 1;

PROCESS(sequencer_process, "Light Sequencer");
AUTOSTART_PROCESSES(&sequencer_process);

PROCESS_THREAD(sequencer_process, ev, data)
{
    static struct etimer timer;
    
    PROCESS_BEGIN();

    simple_udp_register(&udp_conn, UDP_PORT, NULL, UDP_PORT, NULL);
    printf("Light Sequencer Started\n");

    etimer_set(&timer, 3 * CLOCK_SECOND);

    while(1) {
        PROCESS_WAIT_EVENT_UNTIL(ev == PROCESS_EVENT_TIMER);
        
        // Send ACTIVATE command for current light
        uip_ipaddr_t dest;
        uip_ip6addr(&dest, 0xff02,0,0,0,0,0,0,1);
        
        char msg[32];
        snprintf(msg, sizeof(msg), "ACTIVATE %d", current_light);
        simple_udp_sendto(&udp_conn, msg, strlen(msg), &dest);
        
        printf(">>> Activating Light %d for 3 seconds\n", current_light);
        
        // Move to next light
        current_light++;
        if(current_light > 17) {
            current_light = 1;
            printf("=== Restarting sequence ===\n");
        }
        
        etimer_reset(&timer);
    }

    PROCESS_END();
}
