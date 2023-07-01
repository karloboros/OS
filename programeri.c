#include <stdio.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <pthread.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>

#define GG 3

pthread_mutex_t m;
pthread_cond_t r[2];
int n;
int brojac[2], uRedu[2], gotovi[2];

char* ispis[] = { "microsoft", "linux" };


void ulazak(int vrsta, int id) {
    pthread_mutex_lock(&m);
    uRedu[vrsta]++;
    while ((gotovi[vrsta] > GG && uRedu[1 - vrsta] > 0) || brojac[1 - vrsta] > 0) 
        pthread_cond_wait(&r[vrsta], &m);
    brojac[vrsta]++;
    uRedu[vrsta]--;
    gotovi[1 - vrsta] = 0;
    printf("Ulazi %s %d\n", ispis[vrsta], id);
    gotovi[vrsta]++;
    pthread_mutex_unlock(&m);
}

void izlazak(int vrsta, int id) {
    pthread_mutex_lock(&m);
    brojac[vrsta]--;
    printf("Izlazi %s %d\n", ispis[vrsta], id);
    if (brojac[vrsta] == 0)
        pthread_cond_broadcast(&r[1 - vrsta]);
    pthread_mutex_unlock(&m);
}


void* programer(void* vrsta) {
    int randomVrsta = rand() & 1;
    int id = *((int*)vrsta) + 1;

    ulazak(randomVrsta, id);
    sleep(4);
    izlazak(randomVrsta, id);
    return NULL;
}



int main() {

    pthread_mutex_init(&m, NULL);
    pthread_cond_init(&r[0], NULL);
    pthread_cond_init(&r[1], NULL);

    printf("Unesite broj programera koji dolaze u kantinu: ");
    scanf("%d", &n);
    pthread_t thr[n];
    srand(time(NULL));
    for (int i = 0; i < n; i++) {
        pthread_create(&thr[i], NULL, programer, &i);
        sleep(2);
    }
    for (int i = 0; i < n; i++) pthread_join(thr[i], NULL);

    pthread_mutex_destroy(&m);
    pthread_cond_destroy(&r[0]);
    pthread_cond_destroy(&r[1]);

    return 0;
}