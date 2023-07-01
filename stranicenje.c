// gcc stranicenje.c -o stranicenje
// ./stranicenje
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>

int t = 0;

int main(int argc, char** argv) {
    setlinebuf(stdout);

    int proces, br_okvir;
    proces = atoi(argv[1]);
    br_okvir = atoi(argv[2]);
    // proces = 2;
    // br_okvir = 1;

    int disk[proces][16][64];
    int okvir[br_okvir][64];
    int tablica[proces][16]; // tablica stranicenja

    // simulacija zadanog broja procesa
    for (int i = 0; i < proces; i++) {
        for (int j = 0; j < 16; j++) {
            for (int k = 0; k < 64; k++) {
                disk[i][j][k] = 0;
            }
        }
    }

    for (int i = 0; i < proces; i++) {
        for (int j = 0; j < 16; j++) {
            tablica[i][j] = 0;
        }
    }

    for (int i = 0; i < br_okvir; i++) {
        for (int j = 0; j < 64; j++) {
            okvir[i][j] = 0;
        }
    }

    srand(time(NULL));

    while (1) {

        for (int i = 0; i < proces; i++) {

            printf("-----------------\n");
            printf("proces: %d\n", i);
            printf("        t: %d\n", t);

            int logAddr = (rand() % 0x03FF);
            logAddr &= 0x3FE;
            // int logAddr = 0x01fe;

            printf("        log. adresa: 0x%04x\n", logAddr);

            //trazi iz tablice stranicenja

            int idxTablica = (logAddr & 0x3c0) >> 6;

            int zapisTablica = tablica[i][idxTablica];
            zapisTablica = zapisTablica & 0xffe0;
            zapisTablica |= t;


            if (t >= 31) {
                t = 0;
                for (int j = 0; j < proces; j++) {
                    for (int k = 0; k < 16; k++) {
                        tablica[j][k] &= 0xffe0;
                    }
                }
                zapisTablica = (zapisTablica & 0xffe0) + 1;
            }

            // nema ga => 
            if ((zapisTablica & 0x20) == 0) {
                printf("        Promasaj!\n");

                //trazenje prvog slobodnog okvira
                bool zauzetiOkviri[br_okvir];
                for (int i = 0; i < br_okvir; i++) {
                    zauzetiOkviri[i] = false;
                }

                for (int j = 0; j < proces; j++) {
                    for (int k = 0; k < 16; k++) {
                        if ((tablica[j][k] & 0x20) != 0) {
                            int var = tablica[j][k];
                            var &= 0xffc0 >> 6;
                            zauzetiOkviri[var] = true;
                        }
                    }
                }
                bool pun = false;
                int dodijeljen_okvir = br_okvir + 1;

                for (int j = 0; j < br_okvir; j++) {
                    pun = false;
                    if (zauzetiOkviri[j] == false) {
                        dodijeljen_okvir = j;
                        break;
                    }
                    pun = true;
                }

                int lruIndeks;
                int zapisLRU = 0x7fffffff;
                int procesIndeks;
                //LRU
                if (pun) {
                    for (int j = 0; j < proces; j++) {
                        for (int k = 0; k < 16; k++) {
                            if ((tablica[j][k] & 0x20) != 0) {

                                int var_tab = tablica[j][k] & 0x1f;
                                int var_lru = zapisLRU & 0x1f;

                                if (var_tab < var_lru) {
                                    zapisLRU = tablica[j][k];
                                    procesIndeks = j;
                                    lruIndeks = k;
                                }
                            }
                        }
                    }

                    dodijeljen_okvir = zapisLRU & 0xffc0;
                    dodijeljen_okvir /= 2 ^ 6;
                    printf("                Izbacujem stranicu 0x%04x iz procesa %d\n", (lruIndeks << 6), procesIndeks);
                    printf("                LRU izbacene stranice: 0x%04x\n", zapisLRU & 0x1f);

                    for (int j = 0; j < 64; j++) {
                        disk[i][lruIndeks][j] = okvir[dodijeljen_okvir][j];
                    }

                    tablica[procesIndeks][lruIndeks] = zapisLRU & 0xffdf;

                }

                printf("                dodijeljen okvir 0x%04x\n", dodijeljen_okvir);

                // load sadrzaja diska
                for (int j = 0; j < 64; j++) {
                    okvir[dodijeljen_okvir][j] = disk[i][idxTablica][j];
                }

                zapisTablica = zapisTablica | 0x20;
                zapisTablica = zapisTablica & 0x003f;
                zapisTablica = zapisTablica | (dodijeljen_okvir << 6);


            }

            tablica[i][idxTablica] = zapisTablica;

            int okvirIndeks = (zapisTablica & 0xffc0);
            int pomak = (logAddr & 0x3f);
            int fizAdresa = okvirIndeks | pomak;

            printf("        fiz. adresa: 0x%04x\n", fizAdresa);
            printf("        zapis tablice: 0x%04x\n", zapisTablica);
            printf("        sadrzaj adrese: %d\n", okvir[okvirIndeks][pomak]++);

            t++;
            sleep(2);
        }
    }
    return 0;
}