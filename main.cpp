#include <cstdlib>
#include <cstdio>
#include <cassert>
#include <cstring>
#include <cctype>

#include "../Tree/Tree.h"


struct operation_t {
    int priority;
    int argnum;
    char *operation;
    char *latexPrefix;
    char *latexInfix;
    char *latexSuffix;
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

operation_t *makeOperation(int priority, int argnum, char *operation, char *latexPrefix, char *latexInfix, char *latexSuffix, double value) {
    operation_t *op = (operation_t *) calloc(1, sizeof(operation_t));

    op->priority = priority;
    op->argnum = argnum;
    op->operation = operation;
    op->latexInfix = latexInfix;
    op->latexPrefix = latexPrefix;
    op->latexSuffix = latexSuffix;
    op->value = value;

    return op;
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

void loadSubequation(node_t *node, char *serialized) {
    serialized = strchr(serialized, '(') + 1;
    char *begin = serialized;
    serialized = strchr(serialized, '(') + 1;

    int level = 1;
    while(level) {
        if(*serialized == '(')
            level++;
        else if (*serialized == ')')
            level--;
        serialized++;
    }
    char *lexemeBegin = serialized + 1;
    char *lexemeEnd = strchr(lexemeBegin, ')');

    while (isblank(*lexemeBegin ))
        lexemeBegin++;

    while(isblank(*(lexemeEnd-1)))
        lexemeEnd--;

    char oldLexeme = *lexemeEnd;

    *lexemeEnd = '\0';

#define DEF_OP(ID, ARGNUM, PRIORITY, LEXEME, LATEX_PREFIX, LATEX_INFIX, LATEX_SUFFIX, EXEC_OPERATION, DERIVATIVE) {\
    if(strcmp(LEXENE, lexemeBegin) == 0) { \
        operation_t *op = makeOperation(PRIORITY, ARGNUM, OPERATION, LATEX_PREFIX, LATEX_INFIX, LATEX_SUFFIX, 0);\
    }\
    else \
}\

#include "operations.h"
    {
        double val = 0;
        sscanf(lexemeBegin, "%lf", &val);
        operation_t *op = makeOperation(1000, 0, "", "", "", "", val);
    }

#undef DEF_OP
    *lexemeEnd = oldLexeme;
    *lexemeBegin = '\0';
    node->value = op;
}

tree_t *loadEquation(char *filename) {
    char *serialized = loadFile(filename);
    tree_t *parsed = makeTree(nullptr);
    loadSubequation(parsed->head, nullptr);
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

