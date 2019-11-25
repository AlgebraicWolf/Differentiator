#include <cstdlib>
#include <cstdio>
#include <cassert>
#include <cstring>
#include <cctype>

#include "../Tree/Tree.h"

enum OP_TYPE {
    VAR, // x
    CONST, // 1
    UNARY_FUNC, // sin(x)
    BINARY, // 2 * 3
    BINARY_FUNC //pow(x, x)
};

struct operation_t {
    OP_TYPE type;
    char *operation;
    char *latexPrefix;
    char *latexInfix;
    char *latexSuffix;
    void *exec;
    double value;
};

node_t *getP(const char **e);

node_t *getV(const char **e);

node_t *getN(const char **e);

node_t *getE(const char **e);

node_t *getT(const char **e);

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
                    0, 0, "/",   "", 0
            },
            {
                    0, 0, "",    "", 10
            },
            {
                    0, 0, "",    "", 20
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

operation_t *
makeOperation(OP_TYPE type, char *operation, char *latexPrefix, char *latexInfix, char *latexSuffix, void *exec,
              double value) {
    operation_t *op = (operation_t *) calloc(1, sizeof(operation_t));

    op->type = type;
    op->operation = operation;
    op->latexInfix = latexInfix;
    op->latexPrefix = latexPrefix;
    op->latexSuffix = latexSuffix;
    op->exec = exec;
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
        fprintf(output, "%lf", ((operation_t *) subeq->value)->value);
    } else {
        if (subeq->left) {
            saveSubequation(subeq->left, output);
        }

        fprintf(output, "%s", ((operation_t *) subeq->value)->operation);

        if (subeq->right) {
            saveSubequation(subeq->right, output);
        }
    }
    fprintf(output, "%c", ')');
}

void skipWhitespaces(const char **e) {
    assert(e);
    assert(*e);
    while (isblank(**e) && **e)
        (*e)++;
}

void nextChar(const char **e) {
    skipWhitespaces(e);
    (*e)++;
    skipWhitespaces(e);
}

void skipChars(const char **e, int n) {
    for (int i = 0; i < n; i++)
        nextChar(e);
}

node_t *getN(const char **e) {
    assert(e);
    assert(*e);

    int scanned = 0;
    double number = 0;
    sscanf(*e, "%ld%n", &number, &scanned);
    if (!scanned)
        return nullptr;

    skipChars(e, scanned);
    operation_t *op = makeOperation(CONST, nullptr, nullptr, nullptr, nullptr, nullptr, number);
    return makeNode(nullptr, nullptr, nullptr, op);
}

node_t *getV(const char **e) {
    assert(e);
    assert(*e);

    int len = 0;
    while (isalpha(*(*e + len))) {
        len++;
    }

    if (!len)
        return nullptr;

    char *subst = (char *) calloc(len, sizeof(char));
    memcpy(subst, e, len);
    (*e) += len;

    operation_t *var = makeOperation(VAR, subst, nullptr, nullptr, nullptr, nullptr, 0);
    node_t *node = makeNode(nullptr, nullptr, nullptr, var);

    return node;
}

node_t *getP(const char **e) {
    assert(e);
    assert(*e);

    if (**e == '(') {
        nextChar(e);
        node_t *subtree = getE(e);
        skipWhitespaces(e);
        if (**e == ')')
            return subtree;
        return nullptr;
    } else if (isdigit(**e)) {
        return getN(e);
    } else if (isalpha(**e)) {
        node_t *subtree = getE(e);
        if (!subtree)
            return nullptr;
        if (**e == '(') {
            nextChar(e);
            node_t *firstOp = getE(e);
            if (!firstOp)
                return nullptr;

            skipWhitespaces(e);
            if (**e == ')') {
                ((operation_t *) subtree->value)->type = UNARY_FUNC;
                node_t *func = makeNode(nullptr, nullptr, firstOp, subtree); //ADD FUNCTION PARSING
                firstOp->parent = func;
                return func;
            } else if (**e == ',') {
                ((operation_t *) subtree->value)->type = BINARY_FUNC;
                nextChar(e);
                node_t *secondOp = getE(e);
                skipWhitespaces(e);
                if (**e != ')' || !secondOp)
                    return nullptr;
                nextChar(e);
                node_t *func = makeNode(nullptr, firstOp, secondOp, subtree); // ADD FUNCTION PARSING
                firstOp->parent = func;
                secondOp->parent = func;
                return func;
            } else {
                return nullptr;
            }

        }
    } else {
        return nullptr;
    }
}

node_t *getT(const char **e) {
    assert(e);
    assert(*e);
}

tree_t *loadEquation(char *filename) {
    char *serialized = loadFile(filename);
    tree_t *parsed = makeTree(nullptr);
    //loadSubequation(parsed->head, nullptr);
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

