#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
    if (argc == 1) {
        printf("usage: zap [] target_file");
    }

    FILE *file = fopen("arg", "r+b");
    if (file == NULL) {
        printf("No such file\n"); return 1;
    }

    char ln[64];
    while (fgets(ln, sizeof(ln), file) != NULL) {

    }
    fclose(file);

    return 0;
}