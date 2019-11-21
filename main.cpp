#include <cstdlib>
#include <cstdio>
#include <cassert>
#include <cstring>

#include "../Tree/Tree.h"


struct operation {
    int priority;

    char *operation;
    char *latex;
};

char *loadFile(const char *filename);

size_t getFilesize(FILE *f);

int main() {
    printf("Loading equation...\n");
    return 0;
}


tree_t *loadEquation(char *filename) {
    char *serialized = loadFile(filename);

    //Do loading routine
}

size_t getFilesize(FILE *f) {
    assert(f);

    fseek(f, 0, SEEK_END);
    size_t size = ftell(f);
    fseek(f, 0, SEEK_SET);

    return size;
}

char *loadFile(const char *filename) {
    assert(filename);

    FILE *input = fopen(filename, "r");

    size_t size = getFilesize(input);
    char *content = (char *) calloc(size, sizeof(char));
    fread(content, sizeof(char), size, input);

    fclose(input);

    return content;
}

