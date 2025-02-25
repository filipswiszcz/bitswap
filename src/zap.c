#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>
#include <time.h>
#include <limits.h>
#include <errno.h>

#include <sys/stat.h>

// TODO
// concurrent overwriting
// support zapping through ssh tunel

// its not that fucking easy, because there is something called file system journaling
// what if the same filename is used multiple times (store whole path and skip the same ones)

typedef struct filenames filenames;

struct filenames {
    char **names;
    size_t k;
    size_t capacity;
};

int isdir(char *filename) {
    struct stat path;
    if (stat(filename, &path) == 0) {
        if (S_ISDIR(path.st_mode)) return 1;
        else if (S_ISREG(path.st_mode)) return 0;
    }
    return -1;
}

void addfn(filenames *arr, char *name) {
    do {
        if (arr -> k >= arr -> capacity) {
            if (arr -> capacity == 0) arr -> capacity = 64;
            else arr -> capacity *= 2;
            arr -> names = realloc(arr -> names, arr -> capacity * sizeof(*arr -> names));
        }
        arr -> names[arr -> k++] = name;
    } while (0);
}

void getdfiles(filenames *arr, char* name) {
    DIR *dir = opendir(name);
    struct dirent *ent;
    while ((ent = readdir(dir)) != NULL) {
        if (!strcmp(ent -> d_name, ".") || !strcmp(ent -> d_name, "..")) continue;

        size_t ts = snprintf(NULL, 0, "%s/%s", name, ent -> d_name) + 1;
        char *cname = malloc(ts);
        snprintf(cname, ts, "%s/%s", name, ent -> d_name);

        int t = isdir(cname);
        if (t == 1) {
            getdfiles(arr, cname);
        } else if (t == 0) {
            addfn(arr, strdup(cname));
        }
        free(cname);
    }
    closedir(dir);
}

void owrite(FILE *file) {
    int c;
    while ((c = fgetc(file)) != EOF) {
        // fseek(file, -1, SEEK_CUR);
        // int b = rand() & 1;
        // fputc((char) b, file);
        printf("%c", c);
    }
    fflush(file);
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
            int t = isdir(argv[i]);
            if (t == 1) {
                getdfiles(&fns, argv[i]);
            } else if (t == 0) {
                addfn(&fns, argv[i]);
            }
        }
    }

    // TODO
        // **select how many threads should be used
        // find if it is a file or dir
            // if dir, then run overwrite * k for every child
            // else run overwrite * k

    FILE *cfile;
    for (size_t i = 0; i < fns.k; i++) {
        cfile = fopen(fns.names[i], "rb+");
        if (cfile == NULL) {
            printf("zap: Could not open file: %s\n", fns.names[i]); continue;
        }
        owrite(cfile);
        fclose(cfile);
    }

    // printf("size=%zu", fns.k);

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