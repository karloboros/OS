#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <math.h>

int broj = 0;
char obr[] = "obrada.txt";
char sta[] = "status.txt";

FILE* obrada, * status;

void obradi_dogadjaj(int sig) {
    printf("%d\n", broj);
}

void obradi_sigterm(int sig) {
    status = fopen(sta, "w+");
    fprintf(status, "%d", broj);
    fclose(status);
    exit(1);
}

void obradi_sigint(int sig) {
    exit(1);
}

int main() {
    struct sigaction act;
    act.sa_handler = obradi_dogadjaj; // maskiranje signala sigusr1
    sigemptyset(&act.sa_mask); // resetiram da u nekom drugom slucaju ne ostane blokiran sigterm
    sigaddset(&act.sa_mask, SIGTERM); // blokiram sigterm
    act.sa_flags = 0;
    sigaction(SIGUSR1, &act, NULL); // najbitniji dio koji salje signal na obradu
    act.sa_handler = obradi_sigterm; // maskiranje signala sigterm
    sigemptyset(&act.sa_mask);
    sigaction(SIGTERM, &act, NULL);
    act.sa_handler = obradi_sigint; // maskiranje signala sigint
    sigaction(SIGINT, &act, NULL);
    printf("Program s PID=%ld krenuo s radom\n", (long)getpid());

    int random;
    obrada = fopen(obr, "a+");
    status = fopen(sta, "w+");
    fscanf(status, "%d", &broj);
    fclose(status);
    if (broj == 0) {
        while (1) {
            if(fscanf(obrada, "%d", &random) == EOF) break; }
        fclose(obrada);
        broj = sqrt(random);
    }

    status = fopen(sta, "w+");
    fprintf(status, "%d", 0);
    fclose(status);
    
    while (1) {
        broj++;
        random = broj * broj;
        obrada = fopen(obr, "a+");
        fprintf(obrada, "%d\n", random);
        fclose(obrada);
        sleep(5); 
    }
    
    return 0;
}