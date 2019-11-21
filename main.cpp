#include <cstdlib>
#include <cstdio>
#include <cassert>
#include <cstring>

#include "../Tree/Tree.h"


struct operation_t {
    int priority;
    int argnum;
    char *operation;
    char *latex;
    double value;
};

char *loadFile(const char *filename);

size_t getFilesize(FILE *f);

void saveSubequation(node_t *subeq, FILE *output);

void saveEquation(tree_t *eq, const char *filename);

int main() {
    operation_t ops[] = {
            {
                 0, 0, "sin", "", 0
            },
            {
                0, 0, "cos", "", 0
            },
            {
                0, 0, "/", "" , 0
            },
            {
                0, 0, "", "", 10
            },
            {
                0, 0, "", "", 20
            }
    };
    tree_t *example = makeTree(ops + 2);
    addLeftNode(example, example->head, ops + 0);
    addRightNode(example, example->head, ops + 1);

    addRightNode(example, example->head->left, ops + 3);
    addRightNode(example, example->head->right, ops + 4);

    printf("Loading equation...\n");
    saveEquation(example, "test.txt");
    return 0;
}


void saveEquation(tree_t *eq, const char *filename) {
    FILE *output = fopen(filename, "w");
    saveSubequation(eq->head, output);
    fclose(output);
}

void saveSubequation(node_t *subeq, FILE *output) {
    fprintf(output, "%c", '(');
    if (!subeq->left && !subeq->right) {
        fprintf(output, "%lf", ((operation_t *)subeq->value)->value);
    } else {
        if (subeq->left) {
            saveSubequation(subeq->left, output);
        }

            fprintf(output, "%s", ((operation_t *)subeq->value)->operation);

        if (subeq->right) {
            saveSubequation(subeq->right, output);
        }
    }
    fprintf(output, "%c", ')');
}

tree_t *loadEquation(char *filename) {
    char *serialized = loadFile(filename);

    // Do loading routine...
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

