#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// TODO process args
    // support zapping through ssh tunel

// its not that fucking easy, because there is something called file system journaling?

typedef enum {
    // exclude if whole dir?
    SSH,
    DELETE
} flag;

int main(int argc, char *argv[]) {
    if (argc == 1) {
        printf("usage: zap [OPTION] target_file"); return 1;
    }
    char *filename;
    if (!strcmp(argv[1], "--delete")) {
        if (argc > 2) {
            filename = argv[2];
        } else {
            printf("usage: zap [OPTION] target_file"); return 1;
        }
    } else {
        filename = argv[1];
    }

    FILE *file = fopen(filename, "r+b");
    if (file == NULL) {
        printf("No such file\n"); return 1;
    }

    srand(time(NULL));
    char ln[64];
    while (fgets(ln, sizeof(ln), file) != NULL) {
        for (size_t i = 0; i < sizeof(ln); i++) {
            int b = rand() & 1;
            printf("%d", b);
        }
    }
    fclose(file);

    return 0;
}