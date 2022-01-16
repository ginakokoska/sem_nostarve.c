/* mutex-nostarve.c
 * Autor: Gina Kokoska
 * Datum: 09.01.22
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
#include <sys/types.h>
#include <assert.h>

#define MAX_THREADS 700
int old;
int num_threads;
int loops;

typedef struct __ns_mutex_t {
    int ticket;
    int turn;
    sem_t ticket_lock;
    sem_t lock;
} ns_mutex_t;

void ns_mutex_init(ns_mutex_t *m) {
    m->ticket = 0;
    m->turn = 0;

    sem_init(&m->ticket_lock, 0, 1);
    sem_init(&m->lock, 0, 1);
}

/* imitate atomic fetch and add and create tickets, so that each thread will run equally fair)*/
void ns_mutex_acquire(ns_mutex_t *m) {

    sem_wait(&m->ticket_lock);
    old = m->ticket;
    m->ticket = old + 1;

    printf("old = %d\nticket = %d\nturn = %d\n\n", old, m->ticket, m->turn);

    while (m->turn != old) {
        sem_wait(&m->lock);
    }
}

void ns_mutex_release(ns_mutex_t *m) {
    m->turn = m->turn + 1;
    sem_post(&m->ticket_lock);
    sem_post(&m->lock);
    printf("release lock\n");
}


typedef struct __tinfo_t {
    int thread_id;
} tinfo_t;

void *worker(void *arg) {
    ns_mutex_t *m = (ns_mutex_t *) arg;
    ns_mutex_acquire(m); //lock
    printf("thread_id %ld\n", pthread_self());
    ns_mutex_release(m); //unlock


    return NULL;
}


int main(int argc, char *argv[]) {
    int c;
    
    while ((c = getopt (argc, argv, "l:t:")) != -1) {
        switch(c) {
        case 'l':
            loops = atoi(optarg);
            assert(loops > 0);
            break;vi
        case 't':
            num_threads = atoi(optarg);
            assert(num_threads > 0);
            break;
        } 
    }

        
    if (loops == 0 || num_threads == 0) {
        fprintf(stderr, "Enter -l flag for number of loops, -t flag for number of threads!\n");
        exit(1);
    }

    ns_mutex_t m;
    pthread_t p[num_threads];
    tinfo_t t[num_threads];
    ns_mutex_init(&m);

    printf("parent: begin\n");
    


    int i, j;
    for (i = 0; i < loops; i++) {
        for (j = 0; j < num_threads; j++) {
            pthread_create(&p[j], NULL, worker, &m);
        }

        for(j = 0; j < num_threads; j++) {
            pthread_join(p[j], NULL);
        }
    }
    printf("parent: end\n");
    return 0;
}
