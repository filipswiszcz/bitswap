#include <stdio.h>
#include <stdlib.h>

#ifdef _WIN32
    #include <direct.h>
#else
    #include <unistd.h>
#endif

#include <dirent.h>
#include <string.h>
#include <time.h>
#include <limits.h>
#include <pthread.h>
#include <errno.h>

#include <sys/stat.h>

#define MAX_THREADS = 64
#define MAX_TASKS = 65536

typedef struct threadpool_task threadpool_task_t;

struct threadpool_task {
    void (*function) (void*);
    void *arg;
};

typedef struct threadpool threadpool_t;

struct threadpool {
    pthread_t *threads;
    pthread_mutex_t lock;
    pthread_cond_t notif;
    threadpool_task_t *tasks;
    uint16_t threads_k;
    uint16_t running;
    uint16_t tasks_k;
};

typedef enum {
    INVALID = -1,
    LOCK_FAILURE = -2,
    THREAD_FAILURE = -3,
    FULL = -4,
    SHUTDOWN = -5
} threadpool_error_t;

static void *threadpool_thread(void *threadpool);

threadpool_t *tpinit(uint16_t threads, uint16_t tasks) {
    threadpool_t *threadpool;
    if ((threadpool = (threadpool_t*) malloc(sizeof(threadpool_t))) == NULL) {
        // error and free
        return NULL;
    }
    threadpool -> threads_k = 0;
    threadpool -> tasks_k = tasks;

    threadpool -> threads = (pthread_t*) malloc(sizeof(pthread_t) * threads);
    threadpool -> tasks = (threadpool_task_t*) malloc(sizeof(threadpool_task_t) * tasks);

    if ((pthread_mutex_init(&(threadpool -> lock), NULL) != 0) || (pthread_cond_init(&(threadpool -> notif), NULL) != NULL) 
        || (threadpool -> threads == NULL) || (threadpool -> tasks) == NULL) {
            // error and free
            return NULL;
        }

    for (int i = 0; i < threads; i++) {
        if (pthread_create(&(threadpool -> threads[i]), NULL, threadpool_thread, (void*) threadpool) != 0) {
            // error and free
            return NULL;
        }
        threadpool -> threads_k++;
        threadpool -> running++;
    }

    return threadpool;
}

int tpadd(threadpool_t *threadpool) {}

// TODO
// concurrent overwriting
// support zapping through ssh tunel

// problems
// its not that fucking easy, because there is something called file system journaling, which can store versions of file nad u cant fucking access it easily
// what if the same filename is used multiple times (store whole path and skip the same ones)
// size of bytes stays the same, which can be later used in forencis profiling

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

void getdfiles(filenames *dns, filenames *fns, char* name) {
    DIR *dir = opendir(name);
    struct dirent *ent;
    while ((ent = readdir(dir)) != NULL) {
        if (!strcmp(ent -> d_name, ".") || !strcmp(ent -> d_name, "..")) continue;

        size_t ts = snprintf(NULL, 0, "%s/%s", name, ent -> d_name) + 1;
        char *cname = malloc(ts);
        snprintf(cname, ts, "%s/%s", name, ent -> d_name);

        int t = isdir(cname);
        if (t == 1) {
            getdfiles(dns, fns, cname);
            addfn(dns, strdup(cname));
        } else if (t == 0) {
            addfn(fns, strdup(cname));
        }
        free(cname);
    }
    closedir(dir);
}

void owrite(FILE *file) {
    int c;
    while ((c = fgetc(file)) != EOF) {
        fseek(file, -1, SEEK_CUR);
        int b = rand() & 1;
        fputc((char) b, file);
    }
    fflush(file);
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("usage: zz [-d] [-r reiteration] [-t threads] target_file\n       zz [-d] [-r reiteration] [-t threads] target_directory\n"); return 1;
    }
    
    filenames fns = {0}, dns = {0};
    int del = 0, k = 1;
    for (size_t i = 1; i < argc; i++) {
        if (!strcmp(argv[i], "-d") || !strcmp(argv[i], "--delete")) del = 1;
        else if (!strcmp(argv[i], "-r") || !strcmp(argv[i], "--reiteration")) {
            if ((i + 1) > argc) {
                printf("usage: zz [-d] [-r reiteration] [-t threads] target_file\n       zz [-d] [-r reiteration] [-t threads] target_directory\n"); return 1;
            } else {
                char *p;
                int rk = strtol(argv[i + 1], &p, 10);
                if (errno != 0 || *p != '\0') {
                    printf("zz: Reiteration has to be a number\n"); return 1;
                } else if (rk > 8 || rk < 1) {
                    printf("zz: -r: Reiteration has to be in range <1, 8>\n"); return 1;
                } else {
                    k = rk; i++;
                }
            }
        } else if (!strcmp(argv[i], "-t") || !strcmp(argv[i], "--threads")) {
            if ((i + 1) > argc) {

            } else {
                char *p;
                int rk = strtol(argv[i + 1], &p, 10);
                if (errno != 0 || *p != '\0') {
                    printf("zz: Threads have to be a number\n"); return 1;
                } else if (rk > 8 || rk < 1) {
                    printf("zz: -t: Threads have to be in range <1, 8>\n"); return 1;
                } else {
                    k = rk; i++;
                }
            }
        } else {
            int t = isdir(argv[i]);
            if (t == 1) {
                getdfiles(&dns, &fns, argv[i]);
                addfn(&dns, argv[i]);
            } else if (t == 0) {
                addfn(&fns, argv[i]);
            } else {
                printf("zz: %s: No such file or directory\n", argv[i]); return 1;
            }
        }
    }

    srand(time(NULL));
    FILE *cfile;
    for (size_t i = 0; i < fns.k; i++) {
        cfile = fopen(fns.names[i], "rb+");
        if (cfile == NULL) {
            printf("zz: Could not open file: %s\n", fns.names[i]); continue;
        }
        if (k > 1) {
            for (size_t j = 0; j < k; j++) owrite(cfile);
        } else {
            owrite(cfile);
        }
        fclose(cfile);
        if (del) {
            remove(fns.names[i]);
        }
    }

    if (del) {
        for (size_t i = 0; i < dns.k; i++) {
            rmdir(dns.names[i]);
        }
    }

    free(fns.names);
    free(dns.names);

    return 0;
}