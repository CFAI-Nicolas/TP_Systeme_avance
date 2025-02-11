#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include <sys/time.h>

#define SIZE (int)1e7
static int tab[SIZE];
static int globalMin;
static int globalMax;

pthread_mutex_t lock_minmax = PTHREAD_MUTEX_INITIALIZER;

double get_time() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (double)tv.tv_sec + (double)tv.tv_usec / 1e6;
}

typedef struct {
    int debut;
    int fin;
} ArgThread;

void* thread_func(void* arg) {
    ArgThread* data = (ArgThread*)arg;

    int localMin = tab[data->debut];
    int localMax = tab[data->debut];

    for (int i = data->debut + 1; i < data->fin; i++) {
        if (tab[i] < localMin) localMin = tab[i];
        if (tab[i] > localMax) localMax = tab[i];
    }

    pthread_mutex_lock(&lock_minmax);
    if (localMin < globalMin) globalMin = localMin;
    if (localMax > globalMax) globalMax = localMax;
    pthread_mutex_unlock(&lock_minmax);

    pthread_exit(NULL);
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <nb_threads>\n", argv[0]);
        return EXIT_FAILURE;
    }

    int nb_threads = atoi(argv[1]);
    if (nb_threads <= 0) {
        fprintf(stderr, "Le nombre de threads doit être > 0\n");
        return EXIT_FAILURE;
    }

    srand(time(NULL));

    for (int i = 0; i < SIZE; i++) {
        tab[i] = rand() % 1000000;
    }

    globalMin = tab[0];
    globalMax = tab[0];

    double start = get_time();

    pthread_t* th = malloc(nb_threads * sizeof(pthread_t));
    ArgThread* args = malloc(nb_threads * sizeof(ArgThread));

    int chunk = SIZE / nb_threads;
    for (int i = 0; i < nb_threads; i++) {
        args[i].debut = i * chunk;
        args[i].fin = (i == nb_threads - 1) ? SIZE : (i + 1) * chunk;
        
        if (pthread_create(&th[i], NULL, thread_func, &args[i]) != 0) {
            perror("pthread_create");
            free(th);
            free(args);
            return EXIT_FAILURE;
        }
    }

    for (int i = 0; i < nb_threads; i++) {
        pthread_join(th[i], NULL);
    }

    double end = get_time();

    printf("Min = %d, Max = %d\n", globalMin, globalMax);
    printf("Temps (threaded, avec mutex), %d threads : %f s\n", nb_threads, end - start);

    free(th);
    free(args);

    // Destruction du mutex
    pthread_mutex_destroy(&lock_minmax);

    return 0;
}
