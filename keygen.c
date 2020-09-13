#include <time.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
    if (argc < 2){
        fprintf(stderr, "Error: missing keylength argument");
        exit(0);
    }
    srand(time(NULL));
    int keylength = atoi(argv[1]);

    int i = 0;
    for (i = 0; i < keylength; i++){
        char randchar = "ABCDEFGHIJKLMNOPQRSTUVWXYZ "[random () % 27];
        fprintf(stdout, "%c", randchar);
    }

    fprintf(stdout, "\n");

    return 0;
}