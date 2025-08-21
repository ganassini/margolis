// margolis.c
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <time.h>
#include <string.h>
#include <signal.h>
#include <sys/time.h>
#include <errno.h>

#include "config.h"
#include "params.h"

// plane state
typedef enum {
    WAITING_FOR_LANDING,
    DURING_LANDING,
    WAITING_FOR_GATE,
    DURING_DISEMBARK,
    WAITING_FOR_TAKEOFF,
    DURING_TAKEOFF,
    FINISHED,
    CRASHED_STARVATION,
    CRASHED_DEADLOCK
} PlaneState;

// flight type
typedef enum {
    DOMESTIC,
    INTERNATIONAL
} FlightType;

// plane (thread)
typedef struct {
    int         id;
    pthread_t   thread_id;
    FlightType  type;
    PlaneState  state;
    time_t      created_at;
    time_t      waiting_since;
    time_t      finished_at;
    bool        is_in_critical_state;
} Plane;

// airport resources
typedef struct {
    sem_t           tracks;
    sem_t           gates;
    sem_t           tower;
    pthread_mutex_t mutex_common;
    pthread_mutex_t mutex_priority;
    int             waiting_international_flights;
    bool            tower_is_busy;
} Airport;

// statistics
typedef struct {
    int     total_managed_planes;
    int     successfully_managed_planes;
    int     planes_crashed_by_starvation;
    int     planes_crashed_by_deadlock;
    int     deadlocks_detected;
    int     starvation_cases;
    double  average_operation_time;
    int     maximum_simultaneous_planes;
    int     active_planes;
} Statistics;

// global vars
Airport airport;
Statistics statistics = {0};
Plane *planes;
int simulation_is_active = 1;
time_t simulation_start;
pthread_mutex_t mutex_statistics = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_planes = PTHREAD_MUTEX_INITIALIZER;

// utils
int potential_deadlock_detected(int plane_id);
// landing
int try_international_landing(int plane_id);
int try_domestic_landing(int plane_id);
// disembark
int try_international_disembark(int plane_id);
int try_domestic_disembark(int plane_id);
// takeoff
int try_international_takeoff(int plane_id);
int try_domestic_takeoff(int plane_id);
// main plane thread
void *plane_thread(void* arg);
// continuously spawn planes
void *spawn_planes(void* arg);
// final report
void print_final_report();
void open_airport();
// get flight type
const char *get_flight_type(FlightType type);
// cleanup
void cleanup();
// handler de sinal para parada controlada
void sigint_handler(int sig);
// logging
void print_log(int plane_id, const char* operation, const char* details);

int main() {
    // TODO: colors!!
    printf("            :::   :::       :::     :::::::::   ::::::::   ::::::::  :::        ::::::::::: ::::::::    \n");
    printf("      :+:+: :+:+:    :+: :+:   :+:    :+: :+:    :+: :+:    :+: :+:            :+:    :+:    :+:        \n");
    printf("    +:+ +:+:+ +:+  +:+   +:+  +:+    +:+ +:+        +:+    +:+ +:+            +:+    +:+                \n");
    printf("   +#+  +:+  +#+ +#++:++#++: +#++:++#:  :#:        +#+    +:+ +#+            +#+    +#++:++#++          \n");
    printf("  +#+       +#+ +#+     +#+ +#+    +#+ +#+   +#+# +#+    +#+ +#+            +#+           +#+           \n");
    printf(" #+#       #+# #+#     #+# #+#    #+# #+#    #+# #+#    #+# #+#            #+#    #+#    #+#            \n");
    printf("###       ### ###     ### ###    ###  ########   ########  ########## ########### ########              \n");
    printf("\n");
    printf("              M  A  R  G  O  L  I  S\n\n");
    printf("                                                    by Guilherme Ganassini && Gustavo Domenech\n");
    
    // allocate the planes array
    planes = (Plane*)malloc(MAX_N_PLANES * sizeof(Plane));
    if (planes == NULL) {
        perror("--> failed to allocate memory for planes array");
        exit(1);
    }
    // randint seed
    srand(time(NULL));
    
    // change 'ctrl + c' behavior
    signal(SIGINT, sigint_handler);
    
    open_airport();
    simulation_start = time(NULL);
    
    // this thread keeps generates planes after a random interval 
    pthread_t generator_thread;
    if (pthread_create(&generator_thread, NULL, spawn_planes, NULL) != 0) {
        // TODO: colors!!
        perror("--> falha ao criar thread geradora");
        exit(1);
    }
    
    // execute the simulation for the duration specified in 'config.h'
    time_t simulation_duration_limit = simulation_start + SIM_DURATION;
    while (simulation_is_active && time(NULL) < simulation_duration_limit) {
        sleep(1);
        
        // show status every 30 seconds
        if ((time(NULL) - simulation_start) % 30 == 0) {
            // TODO: colors here would be very nice
            printf("\n[STATUS] tempo: %lds | aviões %d criados | %d ativos | %d finalizados |\n\n", 
                   time(NULL) - simulation_start,
                   statistics.total_managed_planes,
                   statistics.active_planes,
                   statistics.successfully_managed_planes);
        }
    }
    
    simulation_is_active = 0;
    // TODO: colors!!
    printf("\n--> limite de tempo da simulação atingido, aguardando finalização das operações...\n");
    

    pthread_join(generator_thread, NULL);
    
    // TODO: colors!!
    printf("--> esperando operações de vôo terminarem...\n");

    time_t timeout = time(NULL) + WAITING_TIMEOUT;
    for (int i = 0; i < statistics.total_managed_planes && i < MAX_N_PLANES; i++) {
        if (time(NULL) > timeout) {
            // TODO: colors!!
            printf("--> limite de tempo alcançado, forçando parada....\n");
            break;
        }
        
        void* result_thread;
        struct timespec timeout_spec;
        timeout_spec.tv_sec = 2; // 2 seconds
        timeout_spec.tv_nsec = 0;
        
        int join_result = pthread_timedjoin_np(planes[i].thread_id, &result_thread, &timeout_spec);
        if (join_result == ETIMEDOUT) {
            // if timeout, normal join (no timeout)
            pthread_join(planes[i].thread_id, NULL);
        }
    }
    
    printf("--> todas as operações finalizadas.\n\n");
    
    print_final_report();
    
    cleanup();
    
    printf("\n--> simulação finalizada\n");

    return 0;
}

// landing operations
int 
try_domestic_landing(int plane_id) 
{
    print_log(plane_id, "POUSO", "solicitando torre");
    
    // check for international flights priority
    pthread_mutex_lock(&airport.mutex_priority);
    while (airport.waiting_international_flights > 0) {
        pthread_mutex_unlock(&airport.mutex_priority);
        usleep(100000); // waits 100ms
        
        // check for starvation
        time_t now = time(NULL);
        int waiting_time = now - planes[plane_id].waiting_since;
        
        if (waiting_time > TIME_TILL_CRASH) {
            // TODO: colors!!
            print_log(plane_id, "STARVATION", "avião caiu após 90s de espera");
            return -1; // starvation crash
        } else if (waiting_time > TIME_TILL_CRITICAL_STATE && !planes[plane_id].is_in_critical_state) {
            print_log(plane_id, "STARVATION", "state crítico - 60s de espera");
            planes[plane_id].is_in_critical_state = 1;
            pthread_mutex_lock(&mutex_statistics);
            statistics.starvation_cases++;
            pthread_mutex_unlock(&mutex_statistics);
        }
        
        pthread_mutex_lock(&airport.mutex_priority);
    }
    pthread_mutex_unlock(&airport.mutex_priority);
    
    // tower -> track
    if (sem_wait(&airport.tower) != 0) return 0;
    
    print_log(plane_id, "POUSO", "torre adquirida, solicitando pista");
    
    if (potential_deadlock_detected(plane_id)) {
        print_log(plane_id, "DEADLOCK", "detectado durante pouso");
        sem_post(&airport.tower);
        return 0;
    }
    
    if (sem_wait(&airport.tracks) != 0) {
        sem_post(&airport.tower);
        return 0;
    }
    
    print_log(plane_id, "POUSO", "recursos adquiridos, iniciando pouso");
    planes[plane_id].state = DURING_LANDING;
    
    // simulate landing duration
    usleep(500000 + rand() % 1000000);
    
    // release resources
    sem_post(&airport.tracks);
    sem_post(&airport.tower);
    
    print_log(plane_id, "POUSO", "concluído com sucesso");
    return 1;
}

int
try_international_landing(int plane_id) 
{
    print_log(plane_id, "POUSO", "solicitando pista");
    
    // track -> tower
    if (sem_wait(&airport.tracks) != 0) return 0;
    
    print_log(plane_id, "POUSO", "pista adquirida, solicitando torre");
    
    // check for deadlock
    if (potential_deadlock_detected(plane_id)) {
        print_log(plane_id, "DEADLOCK", "detectado durante pouso");
        sem_post(&airport.tracks);
        return 0;
    }
    
    if (sem_wait(&airport.tower) != 0) {
        sem_post(&airport.tracks);
        return 0;
    }
    
    print_log(plane_id, "POUSO", "recursos adquiridos, iniciando pouso");
    planes[plane_id].state = DURING_LANDING;
    
    // simulates landing duration
    usleep(500000 + rand() % 1000000); // 0.5 to 1.5 seconds
    
    // release resources
    sem_post(&airport.tracks);
    sem_post(&airport.tower);
    
    print_log(plane_id, "POUSO", "concluído com sucesso");
    return 1;
}

// disembark operations
int
try_international_disembark(int plane_id)
{
    print_log(plane_id, "DESEMBARQUE", "Solicitando portão");
    
    // gate -> tower
    if (sem_wait(&airport.gates) != 0) return 0;
    
    print_log(plane_id, "DESEMBARQUE", "Portão adquirido, solicitando torre");
    
    if (sem_wait(&airport.tower) != 0) {
        sem_post(&airport.gates);
        return 0;
    }
    
    print_log(plane_id, "DESEMBARQUE", "Recursos adquiridos - iniciando desembarque");
    planes[plane_id].state = DURING_DISEMBARK;
    
    // simulates disembark duration
    usleep(1000000 + rand() % 2000000); // 1 to 3 seconds
    
    // release the tower first
    sem_post(&airport.tower);
    
    // keep gate for longer
    usleep(500000);
    sem_post(&airport.gates);
    
    print_log(plane_id, "DESEMBARQUE", "Concluído com sucesso");
    return 1;
}

int
try_domestic_disembark(int plane_id)
{
    print_log(plane_id, "DESEMBARQUE", "solicitando torre");
    
    // check priority
    pthread_mutex_lock(&airport.mutex_priority);
    while (airport.waiting_international_flights > 0) {
        pthread_mutex_unlock(&airport.mutex_priority);
        usleep(100000);
        
        time_t now = time(NULL);
        int waiting_time = now - planes[plane_id].waiting_since;
        
        if (waiting_time > TIME_TILL_CRASH) {
            print_log(plane_id, "STARVATION", "avião caiu após 90s de espera");
            return -1;
        }
        
        pthread_mutex_lock(&airport.mutex_priority);
    }
    pthread_mutex_unlock(&airport.mutex_priority);
    
    // tower -> gate
    if (sem_wait(&airport.tower) != 0) return 0;
    
    print_log(plane_id, "DESEMBARQUE", "torre adquirida, solicitando portão");
    
    if (sem_wait(&airport.gates) != 0) {
        sem_post(&airport.tower);
        return 0;
    }
    
    print_log(plane_id, "DESEMBARQUE", "recursos adquiridos, iniciando desembarque");
    planes[plane_id].state = DURING_DISEMBARK;
    
    usleep(1000000 + rand() % 2000000);
    
    sem_post(&airport.tower);
    usleep(500000);
    sem_post(&airport.gates);
    
    print_log(plane_id, "DESEMBARQUE", "concluído com sucesso");
    return 1;
}

// takeoff
int 
try_international_takeoff(int plane_id)
{
    print_log(plane_id, "DECOLAGEM", "Solicitando portão");
    
    // gate -> track -> tower
    if (sem_wait(&airport.gates) != 0) return 0;
    
    print_log(plane_id, "DECOLAGEM", "portão adquirido, solicitando pista");
    
    if (sem_wait(&airport.tracks) != 0) {
        sem_post(&airport.gates);
        return 0;
    }
    
    print_log(plane_id, "DECOLAGEM", "pista adquirida, solicitando torre");
    
    if (sem_wait(&airport.tower) != 0) {
        sem_post(&airport.tracks);
        sem_post(&airport.gates);
        return 0;
    }
    
    print_log(plane_id, "DECOLAGEM", "recursos adquiridos, iniciando decolagem");
    planes[plane_id].state = DURING_TAKEOFF;
    
    usleep(800000 + rand() % 1200000); // 0.8 to 2 seconds
    
    // release the resources
    sem_post(&airport.tower);
    sem_post(&airport.tracks);
    sem_post(&airport.gates);
    
    print_log(plane_id, "DECOLAGEM", "concluída com sucesso");
    return 1;
}

int
try_domestic_takeoff(int plane_id)
{
    print_log(plane_id, "DECOLAGEM", "solicitando torre");
    
    // check priority
    pthread_mutex_lock(&airport.mutex_priority);
    while (airport.waiting_international_flights > 0) {
        pthread_mutex_unlock(&airport.mutex_priority);
        usleep(100000);
        
        time_t now = time(NULL);
        int waiting_time = now - planes[plane_id].waiting_since;
        
        if (waiting_time > TIME_TILL_CRASH) {
            print_log(plane_id, "STARVATION", "avião caiu após 90s de espera");
            return -1;
        }
        
        pthread_mutex_lock(&airport.mutex_priority);
    }
    pthread_mutex_unlock(&airport.mutex_priority);
    
    // tower -> gate -> track
    if (sem_wait(&airport.tower) != 0) return 0;
    
    print_log(plane_id, "DECOLAGEM", "torre adquirida, solicitando portão");
    
    if (sem_wait(&airport.gates) != 0) {
        sem_post(&airport.tower);
        return 0;
    }
    
    print_log(plane_id, "DECOLAGEM", "portão adquirido, solicitando pista");
    
    if (sem_wait(&airport.tracks) != 0) {
        sem_post(&airport.gates);
        sem_post(&airport.tower);
        return 0;
    }
    
    print_log(plane_id, "DECOLAGEM", "recursos adquiridos, iniciando decolagem");
    planes[plane_id].state = DURING_TAKEOFF;
    
    usleep(800000 + rand() % 1200000);
    
    sem_post(&airport.tower);
    sem_post(&airport.tracks);
    sem_post(&airport.gates);
    
    print_log(plane_id, "DECOLAGEM", "concluída com sucesso");
    return 1;
}

// main plane thread
void*
plane_thread(void* arg)
{
    int plane_id = *((int*)arg);
    int result;
    
    pthread_mutex_lock(&mutex_statistics);
    statistics.active_planes++;
    if (statistics.active_planes > statistics.maximum_simultaneous_planes) {
        statistics.maximum_simultaneous_planes = statistics.active_planes;
    }
    pthread_mutex_unlock(&mutex_statistics);
    
    // update international flight type counter
    if (planes[plane_id].type == INTERNATIONAL) {
        pthread_mutex_lock(&airport.mutex_priority);
        airport.waiting_international_flights++;
        pthread_mutex_unlock(&airport.mutex_priority);
    }
    
    print_log(plane_id, "INICIO", "avião chegando ao aeroporto");
    
    // OPERATION: Landing
    planes[plane_id].state = WAITING_FOR_LANDING;
    planes[plane_id].waiting_since = time(NULL);
    
    if (planes[plane_id].type == INTERNATIONAL) {
        result = try_international_landing(plane_id);
    } else {
        result = try_domestic_landing(plane_id);
    }
    
    if (result == -1) {
        planes[plane_id].state = CRASHED_STARVATION;
        goto finalizacao;
    } else if (result == 0) {
        planes[plane_id].state = CRASHED_DEADLOCK;
        goto finalizacao;
    }
    
    // OPERAÇÃO: Disembark
    planes[plane_id].state = WAITING_FOR_GATE;
    planes[plane_id].waiting_since = time(NULL);
    
    if (planes[plane_id].type == INTERNATIONAL) {
        result = try_international_disembark(plane_id);
    } else {
        result = try_domestic_disembark(plane_id);
    }
    
    if (result == -1) {
        planes[plane_id].state = CRASHED_STARVATION;
        goto finalizacao;
    } else if (result == 0) {
        planes[plane_id].state = CRASHED_DEADLOCK;
        goto finalizacao;
    }
    
    // waits for takeoff
    planes[plane_id].state = WAITING_FOR_TAKEOFF;
    usleep(2000000 + rand() % 3000000); // Espera entre 2-5 segundos
    
    // OPERATION: Takeoff
    planes[plane_id].waiting_since = time(NULL);
    
    if (planes[plane_id].type == INTERNATIONAL) {
        result = try_international_takeoff(plane_id);
    } else {
        result = try_domestic_takeoff(plane_id);
    }
    
    if (result == -1) {
        planes[plane_id].state = CRASHED_STARVATION;
        goto finalizacao;
    } else if (result == 0) {
        planes[plane_id].state = CRASHED_DEADLOCK;
        goto finalizacao;
    }
    
    planes[plane_id].state = FINISHED;
    print_log(plane_id, "SUCESSO", "operações concluídas com sucesso");

finalizacao:
    planes[plane_id].finished_at = time(NULL);
    
    if (planes[plane_id].type == INTERNATIONAL) {
        pthread_mutex_lock(&airport.mutex_priority);
        airport.waiting_international_flights--;
        pthread_mutex_unlock(&airport.mutex_priority);
    }
    
    pthread_mutex_lock(&mutex_statistics);
    statistics.active_planes--;
    
    switch(planes[plane_id].state) {
        case FINISHED:
            statistics.successfully_managed_planes++;
            break;
        case CRASHED_STARVATION:
            statistics.planes_crashed_by_starvation++;
            break;
        case CRASHED_DEADLOCK:
            statistics.planes_crashed_by_deadlock++;
            statistics.deadlocks_detected++;
            break;
        default:
            break;
    }
    pthread_mutex_unlock(&mutex_statistics);
    
    return NULL;
}

// continuously spawn planes
void*
spawn_planes(void* arg)
{
    int plane_counter = 0;
    
    while (simulation_is_active && plane_counter < MAX_N_PLANES) {
        // lock the planes array mutex so no other thread modifies it
        pthread_mutex_lock(&mutex_planes);
        
        // new plane data
        planes[plane_counter].id = plane_counter;
        planes[plane_counter].type = (rand() % 100 < AIRPORT.international_flights_percentage) ? INTERNATIONAL : DOMESTIC;
        planes[plane_counter].state = WAITING_FOR_LANDING;
        planes[plane_counter].created_at = time(NULL);
        planes[plane_counter].is_in_critical_state = 0;
        
        // create plane thread
        if (pthread_create(&planes[plane_counter].thread_id, NULL, plane_thread, &planes[plane_counter].id) != 0) {
            perror("Erro ao criar thread do avião");
            pthread_mutex_unlock(&mutex_planes);
            break;
        }
        
        pthread_mutex_lock(&mutex_statistics);
        statistics.total_managed_planes++;
        pthread_mutex_unlock(&mutex_statistics);
         
        print_log(plane_counter, "CRIADO", 
                    (planes[plane_counter].type == INTERNATIONAL) ? 
                    "Voo internacional criado" : "Voo doméstico criado");
        
        plane_counter++;
        pthread_mutex_unlock(&mutex_planes);
        
        // random interval between creating planes
        usleep((1 + rand() % 10) * 1000000);
    }
    
    return NULL;
}

// final report
void print_final_report() {
    // # TODO: colors
    printf("\n*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*\n");
    printf("                    RELATÓRIO FINAL DA SIMULAÇÃO\n");
    printf("*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*\n");
    printf("\n--> RESUMO:\n");
    printf("  total de aviões criados: %d\n",           statistics.total_managed_planes);
    printf("  aviões finalizados com sucesso: %d\n",    statistics.successfully_managed_planes);
    printf("  aviões crashed por starvation: %d\n",     statistics.planes_crashed_by_starvation);
    printf("  aviões crashed por deadlock: %d\n",       statistics.planes_crashed_by_deadlock);
    printf("  máximo de aviões simultâneos: %d\n",      statistics.maximum_simultaneous_planes);
    printf("  aviões ainda ativos: %d\n",               statistics.active_planes);
    
    printf("\n--> PROBLEMAS:\n");
    printf("  casos de starvation: %d\n",               statistics.starvation_cases);
    printf("  deadlocks detectados: %d\n",              statistics.deadlocks_detected);
    
    printf("\n--> TAXAS DE SUCESSO:\n");
    if (statistics.total_managed_planes > 0) {
        printf("  taxa de sucesso: %.2f%%\n", 
               (float)statistics.successfully_managed_planes / statistics.total_managed_planes * 100);
        printf("  taxa de falha por starvation: %.2f%%\n", 
               (float)statistics.planes_crashed_by_starvation / statistics.total_managed_planes * 100);
        printf("  taxa de falha por deadlock: %.2f%%\n", 
               (float)statistics.planes_crashed_by_deadlock / statistics.total_managed_planes * 100);
    }
    
    printf("\n--> ESTADO FINAL:\n");
    int state_counters[10] = {0};
    for (int i = 0; i < statistics.total_managed_planes && i < MAX_N_PLANES; i++) {
        state_counters[planes[i].state]++;
    }
    
    printf("  finalizados: %d\n",                   state_counters[FINISHED]);
    printf("  crashed por starvation: %d\n",        state_counters[CRASHED_STARVATION]);
    printf("  crashed por deadlock: %d\n",          state_counters[CRASHED_DEADLOCK]);
    printf("    ainda aguardando pouso: %d\n",      state_counters[WAITING_FOR_LANDING]);
    printf("    ainda pousando: %d\n",              state_counters[DURING_LANDING]);
    printf("    ainda aguardando portão: %d\n",     state_counters[WAITING_FOR_GATE]);
    printf("    ainda desembarcando: %d\n",         state_counters[DURING_DISEMBARK]);
    printf("    ainda aguardando decolagem: %d\n",  state_counters[WAITING_FOR_TAKEOFF]);
    printf("    ainda decolando: %d\n",             state_counters[DURING_TAKEOFF]);
    printf("\n*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*\n");
}

// opens the airport
void open_airport() {
    //  semaphores
    sem_init(&airport.tracks, 0, N_TRACKS);
    sem_init(&airport.gates, 0, N_GATES);
    sem_init(&airport.tower, 0, N_TOWER_MAX_OPERATIONS);
    
    // mutexes
    pthread_mutex_init(&airport.mutex_common, NULL);
    pthread_mutex_init(&airport.mutex_priority, NULL);
    
    // counters
    airport.waiting_international_flights = 0;
    airport.tower_is_busy = 0;
    
    // TODO: colors!!
    printf("aeroporto aberto:\n");
    printf("  pistas: %d\n", N_TRACKS);
    printf("  portões: %d\n", N_GATES);
    printf("  capacidade da torre: %d operações simultâneas\n", N_TOWER_MAX_OPERATIONS);
    printf("  tempo de simulação: %d segundos\n", SIM_DURATION);
    printf("\n");
}

// get flight type
const char* get_flight_type(FlightType type) {
    return (type == INTERNATIONAL) ? "INTERNACIONAL" : "DOMESTICO";
}
// cleanup
void cleanup() {
    sem_destroy(&airport.tracks);
    sem_destroy(&airport.gates);
    sem_destroy(&airport.tower);
    pthread_mutex_destroy(&airport.mutex_common);
    pthread_mutex_destroy(&airport.mutex_priority);
    pthread_mutex_destroy(&mutex_statistics);
    pthread_mutex_destroy(&mutex_planes);
    free(planes);
}

// utils
int
potential_deadlock_detected(int plane_id)
{
    // are there resources waiting for more than 30s?
    time_t now = time(NULL);
    if (now - planes[plane_id].waiting_since > 30) {
        return 1;
    }
    return 0;
}

// handler to stop creating planes (threads)
void sigint_handler(int sig) {
    // TODO: colors!!
    printf("\nSIGINT recebido. finalizando simulação...\n");
    simulation_is_active = 0;
}

// logging
void print_log(int plane_id, const char* operation, const char* details) {
    // TODO: colors!!
    time_t now = time(NULL);
    printf("[%ld] avião %d (%s): %s - %s\n", 
           now - simulation_start, plane_id, 
           get_flight_type(planes[plane_id].type), operation, details);
    fflush(stdout);
}
