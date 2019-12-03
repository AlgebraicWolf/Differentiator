#include <cstdlib>
#include <cstdio>
#include <cassert>
#include <cstring>
#include <cctype>
#include <cmath>

#include "../Tree/Tree.h"

enum OP_TYPE {
    VAR, // x
    CONST, // 1
    UNARY_FUNC, // sin(x)
    BINARY, // 2 * 3
    BINARY_FUNC //pow(x, x)
};

struct operation_t {
    int parenthesisPriority;
    OP_TYPE type;
    char *operation;
    char *latexPrefix;
    char *latexInfix;
    char *latexSuffix;
    void *exec;
    double value;
};

const int VAR_CONST_PRIORITY = 1;

node_t *getP(const char **e);

node_t *getV(const char **e);

node_t *getN(const char **e);

node_t *getE(const char **e);

node_t *getT(const char **e);

tree_t *getG(const char *e);

char *loadFile(const char *filename);

size_t getFilesize(FILE *f);

void saveSubequation(node_t *subeq, FILE *output);

void saveEquation(tree_t *eq, const char *filename);

void saveLaTeX(tree_t *tree, FILE *f);

void parseFunction(node_t *func);

tree_t *differentiateEquation(tree_t *eq);

char *dumpOperation(void *op) {
    operation_t *o = (operation_t *) op;
    char *buf = (char *) calloc(64, sizeof(char));
    if (o->type == CONST) {
        sprintf(buf, "%s: %lg", "CONST", o->value);
    } else if (o->type == VAR) {
        sprintf(buf, "%s: %s", "VAR", o->operation);
    } else if (o->type == UNARY_FUNC) {
        sprintf(buf, "%s: %s", "UNARY_FUNC", o->operation);
    } else if (o->type == BINARY) {
        sprintf(buf, "%s: %s", "BINARY", o->operation);
    } else if (o->type == BINARY_FUNC) {
        sprintf(buf, "%s: %s", "BINARY_FUNC", o->operation);
    }
    return buf;
}

int main() {
    char *eq = loadFile("input.txt");
    tree_t *eqTree = getG(eq);
    saveEquation(differentiateEquation(eqTree), "dump.txt");
    FILE *tex = fopen("test.tex", "w");
    saveLaTeX(differentiateEquation(eqTree), tex);
    fclose(tex);
    treeDump(eqTree, "dump.dot", dumpOperation);
    return 0;
}

operation_t *
makeOperation(int parenthesisPriority, OP_TYPE type, char *operation, char *latexPrefix, char *latexInfix,
              char *latexSuffix, void *exec,
              double value) {
    operation_t *op = (operation_t *) calloc(1, sizeof(operation_t));

    op->parenthesisPriority = parenthesisPriority;
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
    if (((operation_t *) subeq->value)->type == CONST) {
        fprintf(output, "%lf", ((operation_t *) subeq->value)->value);
    } else if (((operation_t *) subeq->value)->type == VAR) {
        fprintf(output, "%s", ((operation_t *) subeq->value)->operation);
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

void saveNode(node_t *node, FILE *f) {
    assert(node);
    assert(f);

    operation_t *op = (operation_t *) node->value;

    if (node->parent) {
        if ((op->parenthesisPriority) < (((operation_t *) node->parent->value)->parenthesisPriority)) {
            fprintf(f, "\\left(");
        }
    }

    if (op->type == CONST) {
        fprintf(f, "%lg", op->value);
    } else if (op->type == VAR) {
        fprintf(f, "%s", op->operation);
    } else if (op->type == UNARY_FUNC) {
        fprintf(f, "%s", op->latexPrefix);
        saveNode(node->right, f);
        fprintf(f, "%s%s", op->latexInfix, op->latexSuffix);
    } else if (op->type == BINARY_FUNC || op->type == BINARY) {
        fprintf(f, "%s", op->latexPrefix);
        saveNode(node->left, f);
        fprintf(f, "%s", op->latexInfix);
        saveNode(node->right, f);
        fprintf(f, "%s", op->latexSuffix);
    }

    if (node->parent) {
        if (op->parenthesisPriority < ((operation_t *) node->parent->value)->parenthesisPriority) {
            fprintf(f, "\\right)");
        }
    }
}

bool optimizeZeroOneMultiplication(node_t *node) {
    assert(node);
    operation_t *op = (operation_t *) node->value;
    if (op->operation)
        if (strcmp(op->operation, "*") == 0) {
            operation_t *r = (operation_t *) node->right->value;
            operation_t *l = (operation_t *) node->left->value;
            if (r->value == 1 && r->type == CONST) {
                node->value = node->left->value;
                node->right = node->left->right;
                node->left = node->left->left;
                return true;
            } else if (l->value == 1 && l->type == CONST) {
                node->value = node->right->value;
                node->left = node->right->left;
                node->right = node->right->right;
                return true;
            } else if (l->value == 0 && l->type == CONST) {
                node->value = node->left->value;
                node->left = nullptr;
                node->right = nullptr;
                return true;
            } else if (r->value == 0 && r->type == CONST) {
                node->value = node->left->value;
                node->left = nullptr;
                node->right = nullptr;
            }
        }
    bool flag = false;

    if (node->left)
        flag += optimizeZeroOneMultiplication(node->left);

    if (node->right)
        flag += optimizeZeroOneMultiplication(node->right);

    return flag;
}

bool optimizeZeroAddition(node_t *node) {
    assert(node);
    operation_t *op = (operation_t *) node->value;

    if (op->operation)
        if (strcmp(op->operation, "+") == 0) {
            operation_t *r = (operation_t *) node->right->value;
            operation_t *l = (operation_t *) node->left->value;
            if (r->value == 0 && r->type == CONST) {
                node->value = node->left->value;
                node->right = node->left->right;
                node->left = node->left->left;
                return true;
            } else if (l->value == 0 && l->type == CONST) {
                node->value = node->right->value;
                node->left = node->right->left;
                node->right = node->right->right;
                return true;
            }
        }
    bool flag = false;

    if (node->left)
        flag += optimizeZeroAddition(node->left);

    if (node->right)
        flag += optimizeZeroAddition(node->right);

    return flag;
}

bool optimizeCalculations(node_t *node) {
    assert(node);
    operation_t *op = (operation_t *) node->value;
    switch (op->type) {
        case BINARY_FUNC:
        case BINARY: {
            operation_t *firstOperand = (operation_t *) node->left->value;
            operation_t *secondOperand = (operation_t *) node->right->value;
            if (firstOperand->type == CONST && secondOperand->type == CONST) {
                firstOperand->value = ((double (*)(double, double)) op->exec)(firstOperand->value,
                                                                              secondOperand->value);
                node->value = (void *) firstOperand;
                node->left = nullptr;
                node->right = nullptr;
                return true;
            }
        }
            break;

        case UNARY_FUNC: {
            operation_t *operand = (operation_t *) node->right->value;
            if(operand->type == CONST) {
                operand->value = ((double (*) (double)) op->exec) (operand->value);

                node->value = (void *) operand;
                node->left = nullptr;
                node->right = nullptr;
                return true;
            }
        }
            break;
    }
    bool optimized = false;
    if(node->left)
        optimized += optimizeCalculations(node->left);

    if(node->right)
        optimized += optimizeCalculations(node->right);

    return optimized;
}

void saveLaTeX(tree_t *tree, FILE *f) {
    assert(tree);
    assert(f);
    node_t *node = tree->head;

    fprintf(f, "\\begin{equation}\n");
    saveNode(node, f);
    fprintf(f, "\n\\end{equation}");
}

operation_t *copyOp(operation_t *op) {
    return makeOperation(op->parenthesisPriority, op->type, op->operation, op->latexPrefix, op->latexInfix,
                         op->latexSuffix, op->exec, op->value);
}

node_t *copySubtree(node_t *subtree) {
    assert(subtree);
    node_t *leftSubtree = nullptr;
    node_t *rightSubtree = nullptr;

    if (subtree->left)
        leftSubtree = copySubtree(subtree->left);

    if (subtree->right)
        rightSubtree = copySubtree(subtree->right);


    node_t *copy = makeNode(nullptr, leftSubtree, rightSubtree, copyOp((operation_t *) subtree->value));
    if (rightSubtree)
        rightSubtree->parent = copy;

    if (leftSubtree)
        leftSubtree->parent = copy;

    return copy;
}

operation_t *makeOperationByName(const char *op) {
    assert(op);

    operation_t *operation = nullptr;
#define DEF_LAST_OP(PRIOR, TYPE, OPERATION, LATEX_PREFIX, LATEX_INFIX, LATEX_SUFFIX, EXEC_OPERATION, DERIVATIVE) \
        if(strcmp(OPERATION, op) == 0) { \
            return makeOperation (PRIOR, TYPE, OPERATION, LATEX_PREFIX, LATEX_INFIX, LATEX_SUFFIX, EXEC_OPERATION, 0); \
        }
#define DEF_FIRST_OP(PRIOR, TYPE, OPERATION, LATEX_PREFIX, LATEX_INFIX, LATEX_SUFFIX, EXEC_OPERATION, DERIVATIVE) \
        if(strcmp(OPERATION, op) == 0) { \
            return makeOperation (PRIOR, TYPE, OPERATION, LATEX_PREFIX, LATEX_INFIX, LATEX_SUFFIX, EXEC_OPERATION, 0); \
        }
#define DEF_OP(PRIOR, TYPE, OPERATION, LATEX_PREFIX, LATEX_INFIX, LATEX_SUFFIX, EXEC_OPERATION, DERIVATIVE) \
        if(strcmp(OPERATION, op) == 0) { \
            return makeOperation (PRIOR, TYPE, OPERATION, LATEX_PREFIX, LATEX_INFIX, LATEX_SUFFIX, EXEC_OPERATION, 0); \
        }

#include "operations.h"

#undef DEF_FIRST_OP
#undef DEF_LAST_OP
#undef DEF_OP
}

node_t *differentiateNode(node_t *opNode) {
    assert(opNode);
    operation_t *op = (operation_t *) opNode->value;


    switch (op->type) {
        case CONST: {
            operation_t *newOp = makeOperation(VAR_CONST_PRIORITY, CONST, nullptr, nullptr, nullptr, nullptr, nullptr,
                                               0);
            return makeNode(nullptr, nullptr, nullptr, newOp);
        }

        case VAR: {
            operation_t *newOp = makeOperation(VAR_CONST_PRIORITY, CONST, nullptr, nullptr, nullptr, nullptr, nullptr,
                                               1);
            return makeNode(nullptr, nullptr, nullptr, newOp);
        }

        default: {
#define CONST(x) makeNode(nullptr, nullptr, nullptr, makeOperation(VAR_CONST_PRIORITY, CONST, nullptr, nullptr, nullptr, nullptr, nullptr, (x)))
#define dL differentiateNode(opNode->left)
#define dR differentiateNode(opNode->right)
#define L copySubtree(opNode->left)
#define R copySubtree(opNode->right)
#define DFUNC(op, subtree1, subtree2) makeNode(nullptr, subtree1, subtree2, makeOperationByName(op))
#define FUNC(op, subtree) makeNode(nullptr, nullptr, subtree, makeOperationByName(op))

#define DEF_OP(PRIOR, TYPE, OPERATION, LATEX_PREFIX, LATEX_INFIX, LATEX_SUFFIX, EXEC_OPERATION, DERIVATIVE)  if(strcmp(OPERATION, op->operation) == 0) return DERIVATIVE;
#define DEF_FIRST_OP(PRIOR, TYPE, OPERATION, LATEX_PREFIX, LATEX_INFIX, LATEX_SUFFIX, EXEC_OPERATION, DERIVATIVE) if(strcmp(OPERATION, op->operation) == 0) return DERIVATIVE;
#define DEF_LAST_OP(PRIOR, TYPE, OPERATION, LATEX_PREFIX, LATEX_INFIX, LATEX_SUFFIX, EXEC_OPERATION, DERIVATIVE) if(strcmp(OPERATION, op->operation) == 0) return DERIVATIVE;

#include "operations.h"

#undef DEF_OP
#undef DEF_FIRST_OP
#undef DEF_LAST_OP

#undef dL
#undef dR
#undef L
#undef R
#undef DFUNC
#undef FUNC
            break;
        }
    }
}

tree_t *differentiateEquation(tree_t *eq) {
    assert(eq);

    tree_t *derivative = makeTree(nullptr);
    deleteNode(derivative->head);
    derivative->head = differentiateNode(eq->head);

    bool optStep = true;

    while (optStep) {
        optStep = false;
        optStep += optimizeZeroOneMultiplication(derivative->head);
        optStep += optimizeZeroAddition(derivative->head);
        optStep += optimizeCalculations(derivative->head);
    }

    return derivative;
}

node_t *getN(const char **e) {
    assert(e);
    assert(*e);

    int scanned = 0;
    double number = 0;
    sscanf(*e, "%lf%n", &number, &scanned);
    if (!scanned)
        return nullptr;

    skipChars(e, scanned);
    operation_t *op = makeOperation(VAR_CONST_PRIORITY, CONST, nullptr, nullptr, nullptr, nullptr, nullptr, number);
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
    memcpy(subst, *e, len);
    (*e) += len;

    operation_t *var = makeOperation(VAR_CONST_PRIORITY, VAR, subst, nullptr, nullptr, nullptr, nullptr, 0);
    node_t *node = makeNode(nullptr, nullptr, nullptr, var);

    return node;
}

void parseFunction(node_t *func) {
    assert(func);
    operation_t *op = (operation_t *) func->value;

#define DEF_LAST_OP(PRIOR, TYPE, OPERATION, LATEX_PREFIX, LATEX_INFIX, LATEX_SUFFIX, EXEC_OPERATION, DERIVATIVE)
#define DEF_FIRST_OP(PRIOR, TYPE, OPERATION, LATEX_PREFIX, LATEX_INFIX, LATEX_SUFFIX, EXEC_OPERATION, DERIVATIVE)
#define DEF_OP(PRIOR, TYPE, OPERATION, LATEX_PREFIX, LATEX_INFIX, LATEX_SUFFIX, EXEC_OPERATION, DERIVATIVE) \
    if(op->type == TYPE) { \
        if(strcmp(OPERATION, op->operation) == 0) { \
            op->parenthesisPriority = PRIOR; \
            op->latexPrefix = LATEX_PREFIX; \
            op->latexInfix = LATEX_INFIX; \
            op->latexSuffix = LATEX_SUFFIX; \
            op->exec = EXEC_OPERATION; \
        } \
    }

#include "operations.h"

#undef DEF_FIRST_OP
#undef DEF_LAST_OP
#undef DEF_OP

}

node_t *getP(const char **e) {
    assert(e);
    assert(*e);

    if (**e == '(') {
        nextChar(e);
        node_t *subtree = getE(e);
        skipWhitespaces(e);
        if (**e == ')') {
            nextChar(e);
            return subtree;
        }
        return nullptr;
    } else if (isdigit(**e)) {
        return getN(e);
    } else if (isalpha(**e)) {
        node_t *subtree = getV(e);
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
                parseFunction(subtree);
                firstOp->parent = subtree;
                subtree->right = firstOp;
                nextChar(e);
                return subtree;
            } else if (**e == ',') {
                ((operation_t *) subtree->value)->type = BINARY_FUNC;
                nextChar(e);
                node_t *secondOp = getE(e);
                skipWhitespaces(e);
                if (**e != ')' || !secondOp)
                    return nullptr;
                nextChar(e);
                parseFunction(subtree);
                firstOp->parent = subtree;
                secondOp->parent = subtree;
                subtree->left = firstOp;
                subtree->right = secondOp;
                return subtree;
            } else {
                return nullptr;
            }

        }
        return subtree;
    } else {
        return nullptr;
    }
}


node_t *getFirstPriotityOp(char op) {
#define DEF_LAST_OP(PRIOR, TYPE, OPERATION, LATEX_PREFIX, LATEX_INFIX, LATEX_SUFFIX, EXEC_OPERATION, DERIVATIVE)
#define DEF_OP(PRIOR, TYPE, OPERATION, LATEX_PREFIX, LATEX_INFIX, LATEX_SUFFIX, EXEC_OPERATION, DERIVATIVE)
#define DEF_FIRST_OP(PRIOR, TYPE, OPERATION, LATEX_PREFIX, LATEX_INFIX, LATEX_SUFFIX, EXEC_OPERATION, DERIVATIVE) \
    if(*OPERATION == op) { \
        operation_t *oper = makeOperation(PRIOR, TYPE, OPERATION, LATEX_PREFIX, LATEX_INFIX, LATEX_SUFFIX, EXEC_OPERATION, 0); \
        node_t *node = makeNode(nullptr, nullptr, nullptr, oper); \
        return node; \
    }  \
    else

#include "operations.h"

    return nullptr;

#undef DEF_FIRST_OP
#undef DEF_LAST_OP
#undef DEF_OP
}

node_t *getLastPriotityOp(char op) {
#define DEF_FIRST_OP(PRIOR, TYPE, OPERATION, LATEX_PREFIX, LATEX_INFIX, LATEX_SUFFIX, EXEC_OPERATION, DERIVATIVE)
#define DEF_OP(PRIOR, TYPE, OPERATION, LATEX_PREFIX, LATEX_INFIX, LATEX_SUFFIX, EXEC_OPERATION, DERIVATIVE)
#define DEF_LAST_OP(PRIOR, TYPE, OPERATION, LATEX_PREFIX, LATEX_INFIX, LATEX_SUFFIX, EXEC_OPERATION, DERIVATIVE) \
    if(*OPERATION == op) { \
        operation_t *oper = makeOperation(PRIOR, TYPE, OPERATION, LATEX_PREFIX, LATEX_INFIX, LATEX_SUFFIX, EXEC_OPERATION, 0); \
        node_t *node = makeNode(nullptr, nullptr, nullptr, oper); \
        return node; \
    }  \
    else

#include "operations.h"

    return nullptr;

#undef DEF_LAST_OP
#undef DEF_FIRST_OP
#undef DEF_OP
}

void mergeSubtrees(node_t *subtree, node_t *subtree2, node_t *suptree) {
    assert(subtree);
    assert(subtree2);
    assert(suptree);

    subtree->parent = suptree;
    subtree2->parent = suptree;
    suptree->left = subtree;
    suptree->right = subtree2;
}

node_t *getT(const char **e) {
    assert(e);
    assert(*e);

    node_t *subtree = getP(e);
    node_t *suptree = nullptr;
    skipWhitespaces(e);
    while ((suptree = getFirstPriotityOp(**e))) {
        nextChar(e);
        node_t *subtree2 = getP(e);
        mergeSubtrees(subtree, subtree2, suptree);
        subtree = suptree;
    }

    return subtree;
}

node_t *getE(const char **e) {
    assert(e);
    assert(*e);

    node_t *subtree = getT(e);
    node_t *suptree = nullptr;
    skipWhitespaces(e);
    while ((suptree = getLastPriotityOp(**e))) {
        nextChar(e);
        node_t *subtree2 = getT(e);
        mergeSubtrees(subtree, subtree2, suptree);
        subtree = suptree;
    }
    return subtree;
}

tree_t *getG(const char *e) {
    assert(e);
    node_t *subtree = getE(&e);
    if (*e != '\0')
        return nullptr;

    tree_t *tree = makeTree(nullptr);
    deleteNode(tree->head);
    tree->head = subtree;

    return tree;
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

