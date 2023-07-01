// gcc vrtuljak.c -o vrtuljak -lm -pthread
// ./vrtuljak

#include <stdio.h>
#include <signal.h>
#include <sys/shm.h>
#include <pthread.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>
#include <semaphore.h>

sem_t* smiju_uci;
sem_t* ulazak;
sem_t* smiju_izaci;
sem_t* izlazak;
int ID1, ID2, ID3, ID4;
int mjesta, posjetitelja;

void posjetitelj(int id) 
{
    sem_wait(smiju_uci);
    printf("%d. osoba ulazi na vrtuljak.\n", id);
    sem_post(ulazak);
    sem_wait(smiju_izaci);
    printf("%d. osoba izlazi sa vrtuljka.\n", id);
    sem_post(izlazak);
}
void vrtuljak() {
    while (1) {
        for (int i = 0; i < posjetitelja; i++)
            sem_wait(ulazak);
        
        printf("\nVoznja pocinje. Uzivajte\n");
        sleep(1);
        printf("\n");
        sleep(1);
        printf("Kraj voznje.\n");

        for (int i = 0; i < posjetitelja; i++)
            sem_post(smiju_izaci);
        for (int i = 0; i < posjetitelja; i++) 
            sem_wait(izlazak);
        for (int i = 0; i < posjetitelja; i++) 
            sem_post(smiju_uci);

        printf("\nVrtuljak je zatvoren. Hvala na dolasku\n\n");
    }
}
void brisi(int sig)
{
    /* oslobađanje zajedničke memorije */
    (void)shmdt((char*)smiju_uci);
    (void)shmctl(ID1, IPC_RMID, NULL);
    (void)shmdt((char*)ulazak);
    (void)shmctl(ID2, IPC_RMID, NULL);
    (void)shmdt((char*)smiju_izaci);
    (void)shmctl(ID3, IPC_RMID, NULL);
    (void)shmdt((char*)izlazak);
    (void)shmctl(ID4, IPC_RMID, NULL);
    exit(0);
}

int main() {

    printf("Dopusten broj mjesta na vrtuljku: ");
    scanf("%d", &posjetitelja);
    printf("Broj posjetitelja u redu: ");
    scanf("%d", &mjesta);
    printf("\n");

    ID1 = shmget(IPC_PRIVATE, sizeof(sem_t), 0600);
    smiju_uci = shmat(ID1, NULL, 0);
    shmctl(ID1, IPC_RMID, NULL);
    sem_init(smiju_uci, 1, posjetitelja);
    ID2 = shmget(IPC_PRIVATE, sizeof(sem_t), 0600);
    ulazak = shmat(ID2, NULL, 0);
    shmctl(ID2, IPC_RMID, NULL);
    sem_init(ulazak, 1, 0);
    ID3 = shmget(IPC_PRIVATE, sizeof(sem_t), 0600);
    smiju_izaci = shmat(ID3, NULL, 0);
    shmctl(ID3, IPC_RMID, NULL);
    sem_init(smiju_izaci, 1, 0);
    ID4 = shmget(IPC_PRIVATE, sizeof(sem_t), 0600);
    izlazak = shmat(ID4, NULL, 0);
    shmctl(ID4, IPC_RMID, NULL);
    sem_init(izlazak, 1, 0); 

    if (fork() == 0) {
        for (int i = 1; i < mjesta+1; i++) {
            if (fork() == 0) {
                posjetitelj(i);
                exit(1);
            }
        }
        exit(1);
    }
    for (int i = 0; i < mjesta + 1; i++) {
        wait(NULL);
    }
    vrtuljak();

    sigset(SIGINT, brisi); // brisanje memorije kod prekida

    sem_destroy(smiju_uci);
    sem_destroy(ulazak);
    sem_destroy(smiju_izaci);
    sem_destroy(izlazak);
    
    shmdt(smiju_uci);
    shmdt(ulazak);
    shmdt(smiju_izaci);
    shmdt(izlazak);

    return 0;
}