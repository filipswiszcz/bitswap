#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <limits.h>
#include <errno.h>

// TODO
// concurrent overwriting
// support zapping through ssh tunel

// its not that fucking easy, because there is something called file system journaling

typedef struct filenames filenames;

struct filenames {
    char **names;
    size_t k;
    size_t capacity;
};

void add_filename(filenames *arr, char *name) {
    do {
        if (arr -> k >= arr -> capacity) {
            if (arr -> capacity == 0) arr -> capacity = 64;
            else arr -> capacity *= 2;
            arr -> names = realloc(arr -> names, arr -> capacity * sizeof(*arr -> names));
        }
        arr -> names[arr -> k++] = name;
    } while (0);
}

void overwrite(FILE *file) {
    int c;
    while ((c = fgetc(file)) != EOF) {
        fseek(file, -1, SEEK_CUR);
        int b = rand() & 1;
        fputc((char) b, file);
        fflush(file);
    }
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("usage: zap [-d] [-r reiteration] target_file\n       zap [-d] [-r reiteration] target_directory\n"); return 1;
    }
    
    filenames fns = {0};
    int del = 0, k = 1;
    for (size_t i = 1; i < argc; i++) {
        if (!strcmp(argv[i], "-d") || !strcmp(argv[i], "--delete")) del = 1;
        else if (!strcmp(argv[i], "-r") || !strcmp(argv[i], "--reiteration")) {
            if ((i + 1) > argc) {
                printf("usage: zap [-d] [-r reiteration] target_file\n       zap [-d] [-r reiteration] target_directory\n"); return 1;
            } else {
                char *p;
                int rk = strtol(argv[i + 1], &p, 10);
                if (errno != 0 || *p != '\0') {
                    printf("zap: Reiteration has to be a number"); return 1;
                } else if (rk > 10 || rk < 1) {
                    printf("zap: -r: Reiteration has to be in range <1, 11)"); return 1;
                } else {
                    k = rk; i++;
                }
            }
        } else {
            // TODO add target to list
            printf("%s", argv[i]);
            add_filename(&fns, argv[i]);
        }
    }

    // TODO for every name
        // **select how many threads should be used
        // find if it is a file or dir
            // if dir, then run overwrite * k for every child
            // else run overwrite * k

    // FILE *file = fopen(filename, "rb+");
    // if (file == NULL) {
    //     printf("zap: %s: No such file or directory", filename); return 1;
    // }

    // srand(time(NULL));
    // int c;
    // while ((c = fgetc(file)) != EOF) {
    //     fseek(file, -1, SEEK_CUR);
    //     int b = rand() & 1;
    //     fputc((char) b, file);
    //     fflush(file);
    // }
    // fclose(file);

    // if (del) {
    //     remove(filename);
    // }

    return 0;
}