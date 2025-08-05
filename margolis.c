// margolis.c
// vim:ts=4
// vim:sw=4
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <pthread.h>
#include <stdint.h>
#include <unistd.h>
#include <semaphore.h>

#define MINUTES(m) (m*60)

typedef enum { DOMESTIC, INTERNATIONAL } FlightType; 
typedef enum { TAKEOFF, LANDING, UNLOADING } Operation;

typedef struct {
    uint16_t timeout;
    uint8_t n_planes;
    uint8_t n_runways;
    uint8_t n_towers;
    uint8_t n_gates;
} Arg;


typedef struct {
    pthread_t id;
    FlightType flight_type;
    Operation operation;
} Plane;

typedef struct {
    Plane *owner;
    bool available;
} Resource;

void alloc_runway(Plane *p, Resource *r);
void alloc_tower(Plane *p, Resource *r);
void alloc_gate(Plane *p, Resource *r);
void free_runway(Plane *p, Resource *r);
void free_tower(Plane *p, Resource *r);
void free_gate(Plane *p, Resource *r);
void usage(char *program_name);

int main(int argc, char *argv[]) 
{   
    if (argc == 1) {
        Arg args = {
            .timeout = MINUTES(5),
            .n_planes = 5,
            .n_runways = 3,
            .n_towers = 1,
            .n_gates = 5
        };
    } else {
        // TODO: ARGBEGIN macro
        usage(argv[0]);
        exit(1);
    }

    // mainloop
    do {
        // TODO: create a thread (plane) with priority DOMESTIC | INTERNATIONAL 
        //       (random?) that requests a (random?) Takeoff, Landing and Disembarkation.
        //
        // TODO: keep track of the state of each resource (available/busy)
        //
    } while (1); // TODO: while (time_elapsed < timeout)
    
    // TODO: wait for the running threads to finish

    return 0;
}

void usage(char *program_name)
{
    printf("usage: %s\n", program_name);
}
