#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <pthread.h>
#include <unistd.h>

int Id; 
int *zajednicka1;
int zajednicka2;

void Radna(void) {
    int i = *zajednicka1;
    int j = zajednicka2;
	while(i == 0 || zajednicka2) {
        sleep(1);
        i=*zajednicka1;
        j=zajednicka2;
    }
    
    printf("RADNA DRETVA: Pročitan broj %d i povećan na %d\n", i++, i);
    zajednicka2 = i;
    *zajednicka1 = 0;
}

void *Izlazna(void *x) {
    int i = 0;
    while(i< *(int *)x) {
        int n = zajednicka2;
        while(n==0)
            n = zajednicka2;

    FILE *datoteka;
    datoteka = fopen("datoteka.txt", "a+");
    fprintf(datoteka, "%d\n", n);
    fclose(datoteka);
    printf("IZLAZNA DRETVA: broj upisan u datoteku %d\n\n", n);
    zajednicka2 = 0;

    i++;
    }
}

void brisi(int sig) {
    (void) shmdt((char *) zajednicka1);
    (void) shmctl(Id, IPC_RMID, NULL);
    exit(0);
}

int main(int argc, char **argv) {
    srand(time(NULL));
    int n = atoi(argv[1]);
    Id = shmget(IPC_PRIVATE, sizeof(int), 0600);
    if (Id == -1)
        exit(1); 
    
    zajednicka1 = (int *) shmat(Id, NULL, 0);

    struct sigaction act;
    act.sa_handler = brisi;
    sigaction(SIGINT, &act, NULL); 

    switch(fork()) {
        case 0: {
            pthread_t thr_id[1];
            if (pthread_create(&thr_id[0], NULL, Izlazna, &n) != 0) {
                printf("neuspjelo stvaranje dretve!\n");
                exit(1);
            }
            else {
                printf("Pokrenuta RADNA DRETVA\n");
                printf("Pokrenuta IZLAZNA DRETVA\n\n");
            }
            for(int i = 0; i<n; i++) 
                Radna();
            pthread_join(thr_id[0], NULL);
            printf("Završila RADNA DRETVA\n");
            printf("Završila IZLAZNA DRETVA\n");
            exit(0);
        }
        case -1: {
            printf("Greska u stvaranju proscesa\n");
        }
        default: {
            printf("Pokrenut ULAZNI PROCES\n");
            for(int i = 0; i<n; i++) {
                sleep(2);
                while(*zajednicka1 != 0)
                    sleep(1);
                *zajednicka1 = rand()%100+1;
                printf("ULAZNI PROCES: broj %d\n", *zajednicka1);
            }
        }
    }    

    (void) wait(NULL);
    printf("Završio ULAZNI PROCES\n");
    brisi(0);
    return 0;
}