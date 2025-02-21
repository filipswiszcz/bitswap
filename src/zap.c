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

    FILE *file = fopen(filename, "rb+");
    if (file == NULL) {
        printf("No such file\n"); return 1;
    }

    srand(time(NULL));
    int c;
    while ((c = fgetc(file)) != EOF) {
        fseek(file, -1, SEEK_CUR);
        int b = rand() & 1;
        fputc((char) b, file);
        fflush(file);
    }
    fclose(file);

    return 0;
}