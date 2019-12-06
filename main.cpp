#include <cstdlib>
#include <cstdio>
#include <cassert>
#include <cstring>
#include <cctype>
#include <cmath>
#include <ctime>
#include <climits>

#include "../Tree/Tree.h"

const char *phrases[19] = {"Путем нехитрых математических преобразований получаем",
                           "Таким образом,",
                           "Нетрудно получить",
                           "Согласно таблице производных",
                           "Далее",
                           "Очевидно, что",
                           "Как говорил Георг Кантор, сущность математики - в ее свободе. Без замедлений применим все необходимые правила",
                           "Ежику понятно, что",
                           "Данное рассказывают еще в детском саду:",
                           "Получим",
                           "Смотрим в книгу, видим",
                           "Ясно как божий день, что",
                           "Без всякого махания руками",
                           "С пустой головой применяем таблицу производных и получаем",
                           "Из вышепреведенного нетрудно получить:",
                           "Все согласятся, что",
                           "Дальнейший переход является упражнением для читателя",
                           "Посредством пристального вглядывания в выражение выводим",
                           "Следовательно"};

const int PHRASE_NUM = 19;

const double EPS = 1e-8;

enum OP_TYPE {
    VAR, // x
    CONST, // 1
    PSEUDOCONST,
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

    union {
        double (*binary)(double, double);

        double (*unary)(double);
    } exec;

    double value;
};

const int VAR_CONST_PRIORITY = 1;

node_t *getP(const char **e, char **vars);

node_t *getV(const char **e);

node_t *getN(const char **e);

node_t *getE(const char **e, char **vars);

node_t *getT(const char **e, char **vars);

tree_t *getG(const char *e, char **vars);

char *loadFile(const char *filename);

size_t getFilesize(FILE *f);

void saveSubequation(node_t *subeq, FILE *output);

void saveEquation(tree_t *eq, const char *filename);

void saveLaTeX(tree_t *tree, FILE *f);

void parseFunction(node_t *func);

void optimizeSubequation(node_t *node);

double executeTree(tree_t *tree, double *vars, char **varnames);

tree_t *differentiateEquation(tree_t *eq, char *var, const char *derivativeSymbol, int dumpLevel, FILE *dump);

char *fprintfPointRepresentation(FILE *f, char *var, double x);

operation_t *makeOperation(int parenthesisPriority, OP_TYPE type, char *operation, char *latexPrefix, char *latexInfix,
                           char *latexSuffix, double (*exec)(double, double),
                           double value);

operation_t *makeOperation(int parenthesisPriority, OP_TYPE type, char *operation, char *latexPrefix, char *latexInfix,
                           char *latexSuffix, double (*exec)(double),
                           double value);

char **findEmpty(char **vars);

void saveNode(node_t *node, FILE *f);

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

int ask(const char *question, int min, int max) {
    int ans = min - 1;
    while (ans < min || ans > max) {
        printf("%s\n>", question);
        scanf("%d", &ans);
    }
    return ans;
}

int main() {
    srand(time(nullptr));
    char *eq = loadFile("input.txt");
    auto vars = (char **) calloc((strchr(eq, '\0') - eq) / 2 + 1, sizeof(char *));
    tree_t *eqTree = getG(eq, vars);
    int len = findEmpty(vars) - vars;
    if (len > 0) {
        int report = ask("Хотите получать подробные отчеты о дифференцировании? (0 - нет, 1 - да)", 0, 1);

        int mode = ask("Шо телать будем? (0 - просто дифференцируем, 1 - разложим по Тейлору)", 0, 1);

        int degree = 0;

        switch (mode) {
            case 0:
                degree = ask("Какого порядка берем производную?", 0, INT_MAX);
                break;

            case 1:
                degree = ask("До какой степени раскладываем?", 0, INT_MAX);
        }

        int derivableVar = 0;
        const char *derivativeSymbol = nullptr;
        if (len > 1) {
            derivativeSymbol = "\\partial ";
            printf("Хмммм, похоже, вы использовали несколько переменных. Будем считать частную производную значит?\nВот список доступных для дифференцирования переменных:\n");
            for (int i = 0; i < len; i++)
                printf("%d: %s\n", i + 1, vars[i]);
            derivableVar = ask("Какую переменную выбираешь?", 1, len) - 1;
        } else {
            derivativeSymbol = "d";
        }
        FILE *difDump = fopen("dif.tex", "w");
        fprintf(difDump, "\\documentclass{article}\n"
                         "\\usepackage[utf8]{inputenc}\n"
                         "\\usepackage[russian]{babel}\n"
                         "\\usepackage{graphicx}\n"
                         "\\usepackage{environ}\n"
                         "\\usepackage{etoolbox}\n"
                         "\\usepackage{amsmath}\n"
                         "\\usepackage{resizegather}\n"
                         "\n"
                         "\\begin{document}\n\n");
        tree_t *derivative = eqTree;
        double *positions = nullptr;
        double *values = nullptr;
        if (mode == 1) {
            printf("Пожалуйста, укажите, в какой точке мне разложить\n");
            positions = (double *) calloc(len, sizeof(double));
            values = (double *) calloc(degree + 1, sizeof(double));
            for (int i = 0; i < len; i++) {
                printf("%s=", vars[i]);
                scanf("%lf", positions + i);
            }
        }

        if(mode == 0)
            printf("Дифференцирую...\n");
        else
            printf("Раскладываю...\n");

        if (mode == 0) {
            fprintf(difDump, "Требуется найти:\n\\begin{gather}\n\\frac{%s^{%d}}{%s{%s}^{%d}}\\left(", derivativeSymbol,
                    degree, derivativeSymbol, vars[derivableVar],
                    degree);
            saveNode(eqTree->head, difDump);
            fprintf(difDump, "\\right)\n\\end{gather}");
        } else if (mode == 1) {
            fprintf(difDump, "Требуется разложить в ряд Тейлора до $o(");
            if (positions[derivableVar] == 0)
                fprintf(difDump, "{%s}^{%d})$", vars[derivableVar], degree);
            else if (positions[derivableVar] < 0)
                fprintf(difDump, "{(%s+%lg)}^{%d})$", vars[derivableVar], -positions[derivableVar], degree);
            else
                fprintf(difDump, "{(%s-%lg)}^{%d})$", vars[derivableVar], positions[derivableVar], degree);

            fprintf(difDump, "\n\\begin{gather}");
            saveNode(eqTree->head, difDump);
            fprintf(difDump, "\n\\end{gather}");
        }

        if(mode == 1) {
            values[0] = executeTree(eqTree, positions, vars);
            fprintf(difDump, "Значение функции в заданной точке: $%lg$ \\\\ \n", values[0]);
        }

        for (int i = 0; i < degree; i++) {
            fprintf(difDump, "Вычисляем\n\\begin{gather}\n\\frac{%s^{%d}}{%s{%s}^{%d}}\\left(", derivativeSymbol,
                    i + 1, derivativeSymbol, vars[derivableVar], i + 1);
            saveNode(eqTree->head, difDump);
            fprintf(difDump, "\\right)=\\frac{%s}{%s%s}\\left(", derivativeSymbol, derivativeSymbol,
                    vars[derivableVar]);
            saveNode(derivative->head, difDump);
            fprintf(difDump, "\\right)\n\\end{gather}\n");
            derivative = differentiateEquation(derivative, vars[derivableVar], derivativeSymbol, report, difDump);
            if (mode == 1) {
                values[i + 1] = executeTree(derivative, positions, vars);
                fprintf(difDump, "Значение производной в заданной точке: $%lg$ \\ \n", values[i + 1]);
            }
        }

        printf("Готово!\n");
        if (mode == 0) {
            fprintf(difDump, "Искомая производная: \n\\begin{gather}\n\\frac{%s^{%d}}{%s{%s}^{%d}}\\left(",
                    derivativeSymbol, degree, derivativeSymbol,
                    vars[derivableVar], degree);
            saveNode(eqTree->head, difDump);
            fprintf(difDump, "\\right)=", vars[derivableVar]);
            saveNode(derivative->head, difDump);
            fprintf(difDump, "\n\\end{gather}\n\\end{document}");
        }
        else if(mode == 1) {
            fprintf(difDump, "Искомое разложение:\n\\begin{gather}\n");
            saveNode(eqTree->head, difDump);
            fprintf(difDump, "=");
            if(values[0] != 0)
                fprintf(difDump, "%lg", values[0]);

            for(int i = 1; i <= degree; i++) {
                if(values[i] != 0) {
                    if(i == 1 && fabs(values[0]) < EPS)
                        fprintf(difDump, "\\frac{{");
                    else if(fabs(values[i] -  1) < EPS)
                        fprintf(difDump, "+\\frac{{");
                    else if(fabs(values[i] + 1) < EPS)
                        fprintf(difDump, "-\\frac{{");
                    else
                        fprintf(difDump, "+\\frac{%lg \\cdot {", values[i]);
                    fprintfPointRepresentation(difDump, vars[derivableVar], positions[derivableVar]);
                    fprintf(difDump, "}^{%d}}{%d!}", i, i);
                }
            }
            fprintf(difDump, "+o({");
            fprintfPointRepresentation(difDump, vars[derivableVar], positions[derivableVar]);
            fprintf(difDump, "}^{%d})\\end{gather}\n\\end{document}", degree);
        }
        fclose(difDump);
    }
    return 0;
}

char *fprintfPointRepresentation(FILE *f, char *var, double x) {
    if(x == 0)
        fprintf(f,"%s", var);
    else if (x < 0)
        fprintf(f, "(%s+%lg)", var, -x);
    else
        fprintf(f, "(%s-%lg)", var, x);
}

char **findEmpty(char **vars) {
    while (*vars != nullptr)
        vars++;
    return vars;
}

char **findString(char **vars, char *str) {
    while (*vars != nullptr) {
        if (strcmp(*vars, str) == 0)
            return vars;
        vars++;
    }
    return vars;
}

void addVar(char **vars, char *var) {
    char **pos = findString(vars, var);
    if (*pos == nullptr);
    *pos = var;
}

operation_t *
makeOperation(int parenthesisPriority, OP_TYPE type, char *operation, char *latexPrefix, char *latexInfix,
              char *latexSuffix, double (*exec)(double, double),
              double value) {
    operation_t *op = (operation_t *) calloc(1, sizeof(operation_t));

    op->parenthesisPriority = parenthesisPriority;
    op->type = type;
    op->operation = operation;
    op->latexInfix = latexInfix;
    op->latexPrefix = latexPrefix;
    op->latexSuffix = latexSuffix;
    op->exec.binary = exec;
    op->value = value;

    return op;
}

operation_t *
makeOperation(int parenthesisPriority, OP_TYPE type, char *operation, char *latexPrefix, char *latexInfix,
              char *latexSuffix, double (*exec)(double),
              double value) {
    operation_t *op = (operation_t *) calloc(1, sizeof(operation_t));

    op->parenthesisPriority = parenthesisPriority;
    op->type = type;
    op->operation = operation;
    op->latexInfix = latexInfix;
    op->latexPrefix = latexPrefix;
    op->latexSuffix = latexSuffix;
    op->exec.unary = exec;
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
    } else if (((operation_t *) subeq->value)->type == BINARY_FUNC) {
        fprintf(output, "%s(", ((operation_t *) subeq->value)->operation);
        if (subeq->left) {
            saveSubequation(subeq->left, output);
        }
        fprintf(output, ",");
        if (subeq->right) {
            saveSubequation(subeq->right, output);
        }
        fprintf(output, ")");
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

double executeSubtree(node_t *node, double *vars, char **varnames) {
    assert(node);
    assert(vars);
    assert(varnames);

    auto op = (operation_t *) node->value;

    switch (op->type) {
        case CONST:
            return op->value;
            break;

        case VAR:
        case PSEUDOCONST:
            return vars[findString(varnames, op->operation) - varnames];
            break;

        case UNARY_FUNC:
            return op->exec.unary(executeSubtree(node->right, vars, varnames));
            break;

        case BINARY:
        case BINARY_FUNC:
            return op->exec.binary(executeSubtree(node->left, vars, varnames),
                                   executeSubtree(node->right, vars, varnames));
    }
}

double executeTree(tree_t *tree, double *vars, char **varnames) {
    assert(tree);
    assert(vars);
    assert(varnames);

    return executeSubtree(tree->head, vars, varnames);
}

operation_t *makeMultiplicationOperation() {
    return makeOperation(1, BINARY, "*", "", " \\cdot ", "",
                         (double (*)(double, double)) ([](double a, double b) { return a * b; }), 0);
}

bool optimizeZeroSubstraction(node_t *node) {
    assert(node);
    operation_t *op = (operation_t *) node->value;

    if (op->operation)
        if (strcmp(op->operation, "-") == 0) {
            operation_t *r = (operation_t *) node->right->value;
            operation_t *l = (operation_t *) node->left->value;
            if (r->value == 0 && r->type == CONST) {
                node->value = node->left->value;
                node->right = node->left->right;
                node->left = node->left->left;
                return true;
            } else if (l->value == 0 && l->type == CONST) {
                node->value = makeMultiplicationOperation();
                l->value = -1;
                return true;
            }
        }
    bool flag = false;

    if (node->left)
        flag += optimizeZeroSubstraction(node->left);

    if (node->right)
        flag += optimizeZeroSubstraction(node->right);

    return flag;
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
        if(op->value >= 0)
            fprintf(f, "%lg", op->value);
        else
            fprintf(f, "(%lg)", op->value);
    } else if (op->type == VAR || op->type == PSEUDOCONST) {
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
                if (node->right)
                    node->right->parent = node;

                if (node->left)
                    node->left->parent = node;
                return true;
            } else if (l->value == 1 && l->type == CONST) {
                node->value = node->right->value;
                node->left = node->right->left;
                node->right = node->right->right;
                if (node->right)
                    node->right->parent = node;

                if (node->left)
                    node->left->parent = node;
                return true;
            } else if (l->value == 0 && l->type == CONST) {
                node->value = node->left->value;
                node->left = nullptr;
                node->right = nullptr;
                return true;
            } else if (r->value == 0 && r->type == CONST) {
                node->value = node->right->value;
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

bool optimizeZeroOneDivision(node_t *node) {
    assert(node);
    operation_t *op = (operation_t *) node->value;
    if (op->operation)
        if (strcmp(op->operation, "/") == 0) {
            operation_t *r = (operation_t *) node->right->value;
            operation_t *l = (operation_t *) node->left->value;
            if ((r->value == 1 && r->type == CONST) || (l->value == 0 && l->type == CONST)) {
                node->value = node->left->value;
                node->right = node->left->right;
                node->left = node->left->left;
                if (node->right)
                    node->right->parent = node;

                if (node->left)
                    node->left->parent = node;
                return true;
            }
        }
    bool flag = false;

    if (node->left)
        flag += optimizeZeroOneMultiplication(node->left);

    if (node->right)
        flag += optimizeZeroOneMultiplication(node->right);

    return flag;
}

bool optimizeZeroPower(node_t *node) {
    assert(node);
    operation_t *op = (operation_t *) node->value;

    if (op->operation)
        if (strcmp(op->operation, "pow") == 0) {
            operation_t *r = (operation_t *) node->right->value;
            operation_t *l = (operation_t *) node->left->value;
            if (r->value == 0 && r->type == CONST) {
                node->value = node->right->value;
                node->right = nullptr;
                node->left = nullptr;
                r->value = 1;
                return true;
            } else if (r->value == 1 && r->type == CONST) {
                node->value = node->left->value;
                node->right = node->left->right;
                node->left = node->left->left;
                if (node->right)
                    node->right->parent = node;

                if (node->left)
                    node->left->parent = node;
                return true;
            }
        }
    bool flag = false;

    if (node->left)
        flag += optimizeZeroPower(node->left);

    if (node->right)
        flag += optimizeZeroPower(node->right);

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
            if ((firstOperand->type == CONST) && (secondOperand->type == CONST)) {
                firstOperand->value = op->exec.binary(firstOperand->value, secondOperand->value);
                node->value = (void *) firstOperand;
                node->left = nullptr;
                node->right = nullptr;
                return true;
            }
        }
            break;

        case UNARY_FUNC: {
            operation_t *operand = (operation_t *) node->right->value;
            if (operand->type == CONST) {
                operand->value = op->exec.unary(operand->value);

                node->value = (void *) operand;
                node->left = nullptr;
                node->right = nullptr;
                return true;
            }
        }
            break;
    }
    bool optimized = false;
    if (node->left)
        optimized += optimizeCalculations(node->left);

    if (node->right)
        optimized += optimizeCalculations(node->right);

    return optimized;
}

bool optimizeNearestOperation(node_t *node, char *operation) {
    assert(node);
    operation_t *op = (operation_t *) node->value;
    if (op->type == BINARY)
        if (((operation_t *) node->left->value)->type == BINARY || ((operation_t *) node->right->value)->type == BINARY)
            if (strcmp(op->operation, operation) == 0) {
                operation_t *left = (operation_t *) node->left->value;
                operation_t *right = (operation_t *) node->right->value;
                if (left->type == BINARY)
                    if (strcmp(left->operation, operation) == 0) {
                        operation_t *tmpOp = left;
                        left = right;
                        right = tmpOp;

                        node_t *tmpNode = node->left;
                        node->left = node->right;
                        node->right = tmpNode;
                    }
                if (left->type == CONST && (strcmp(right->operation, operation) == 0)) {
                    operation_t *rightRight = (operation_t *) node->right->right->value;
                    operation_t *rightLeft = (operation_t *) node->right->left->value;

                    if (rightLeft->type == CONST) {
                        left->value = op->exec.binary(left->value, rightLeft->value);
                        node->right = node->right->right;
                        if (node->right->right)
                            node->right->right->parent = node;
                    } else if (rightRight->type == CONST) {
                        left->value = op->exec.binary(left->value, rightRight->value);
                        node->right = node->right->left;
                        if (node->right->left)
                            node->right->left->parent = node;
                    }
                }
            }
    bool optimized = false;
    if (node->left)
        optimized += optimizeNearestOperation(node->left, operation);

    if (node->right)
        optimized += optimizeNearestOperation(node->right, operation);

    return optimized;
}

void saveLaTeX(tree_t *tree, FILE *f) {
    assert(tree);
    assert(f);
    node_t *node = tree->head;

    //fprintf(f, "\\begin{gather}\n");
    saveNode(node, f);
    //fprintf(f, "\n\\end{gather}");
}

operation_t *copyOp(operation_t *op) {
    return makeOperation(op->parenthesisPriority, op->type, op->operation, op->latexPrefix, op->latexInfix,
                         op->latexSuffix, op->exec.binary, op->value);
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

node_t *differentiateNode(node_t *opNode, char *var, const char *derivativeSymbol, int dumpLevel, FILE *dump) {
    assert(opNode);
    operation_t *op = (operation_t *) opNode->value;
    if (op->type == VAR)
        if (strcmp(var, op->operation))
            op->type = PSEUDOCONST;

    if (opNode->left) {
        operation_t *l = (operation_t *) opNode->left->value;
        if (l->type == VAR)
            if (strcmp(l->operation, var))
                l->type = PSEUDOCONST;
    }

    if (opNode->right) {
        operation_t *r = (operation_t *) opNode->right->value;
        if (r->type == VAR)
            if (strcmp(r->operation, var))
                r->type = PSEUDOCONST;
    }

    switch (op->type) {
        case PSEUDOCONST:
        case CONST: {
            operation_t *newOp = makeOperation(VAR_CONST_PRIORITY, CONST, nullptr, nullptr, nullptr, nullptr,
                                               (double (*)(double, double)) nullptr,
                                               0);
            return makeNode(nullptr, nullptr, nullptr, newOp);
        }

        case VAR: {
            operation_t *newOp = makeOperation(VAR_CONST_PRIORITY, CONST, nullptr, nullptr, nullptr, nullptr,
                                               (double (*)(double, double)) nullptr,
                                               1);
            return makeNode(nullptr, nullptr, nullptr, newOp);
        }

        default: {

#define RETURN(X)\
        {\
            node_t *newSubtree = (X); \
            optimizeSubequation(newSubtree); \
            if(dumpLevel == 1) { \
                fprintf(dump, "%s\n", phrases[(int)(PHRASE_NUM * (rand() / (float) RAND_MAX))]); \
                fprintf(dump, "\\begin{gather}\n\\frac{%s}{%s%s}\\left(", derivativeSymbol, derivativeSymbol, var); \
                saveNode(opNode, dump); \
                fprintf(dump, "\\right)="); \
                saveNode(newSubtree, dump); \
                fprintf(dump, "\n\\end{gather}\n"); \
            } \
            return newSubtree;\
        }

#define CONST(x) makeNode(nullptr, nullptr, nullptr, makeOperation(VAR_CONST_PRIORITY, CONST, nullptr, nullptr, nullptr, nullptr, (double (*)(double, double))nullptr, (x)))
#define dL differentiateNode(opNode->left, var, derivativeSymbol, dumpLevel, dump)
#define dR differentiateNode(opNode->right, var, derivativeSymbol, dumpLevel, dump)
#define L copySubtree(opNode->left)
#define R copySubtree(opNode->right)
#define DFUNC(op, subtree1, subtree2) makeNode(nullptr, subtree1, subtree2, makeOperationByName(op))
#define FUNC(op, subtree) makeNode(nullptr, nullptr, subtree, makeOperationByName(op))
#define SPECIAL_CASE(LCASE, RCASE, DERIVATIVE) if((((operation_t *)L->value)->type LCASE) && (((operation_t *)R->value)->type RCASE)) RETURN(DERIVATIVE);
#define ALWAYS(X) RETURN(X)
#define WHATEVER != -1
#define CONSTANT == CONST
#define PSEUDOCONSTANT == PSEUDOCONST


#define DEF_OP(PRIOR, TYPE, OPERATION, LATEX_PREFIX, LATEX_INFIX, LATEX_SUFFIX, EXEC_OPERATION, DERIVATIVE)  if(strcmp(OPERATION, op->operation) == 0)  {DERIVATIVE}
#define DEF_FIRST_OP(PRIOR, TYPE, OPERATION, LATEX_PREFIX, LATEX_INFIX, LATEX_SUFFIX, EXEC_OPERATION, DERIVATIVE) if(strcmp(OPERATION, op->operation) == 0)  {DERIVATIVE}
#define DEF_LAST_OP(PRIOR, TYPE, OPERATION, LATEX_PREFIX, LATEX_INFIX, LATEX_SUFFIX, EXEC_OPERATION, DERIVATIVE) if(strcmp(OPERATION, op->operation) == 0)  {DERIVATIVE}

#include "operations.h"

#undef PSEUDOCONSTANT
#undef RETURN
#undef DEF_OP
#undef DEF_FIRST_OP
#undef DEF_LAST_OP
#undef ALWAYS
#undef SPECIAL_CASE
#undef WHATEVER
#undef CONSTANT
#undef dL
#undef dR
#undef L
#undef R
#undef DFUNC
#undef FUNC
            break;
        }
    }
    if (dumpLevel == 1) {
        printf("\\begin{gather}\n");
        printf("\\begin{gather}\n");
    }
}

void optimizeSubequation(node_t *node) {
    bool optStep = true;

    while (optStep) {
        optStep = false;
        optStep += optimizeZeroOneMultiplication(node);
        optStep += optimizeZeroAddition(node);
        optStep += optimizeCalculations(node);
        optStep += optimizeNearestOperation(node, "*");
        optStep += optimizeNearestOperation(node, "+");
        optStep += optimizeZeroPower(node);
        optStep += optimizeZeroSubstraction(node);
        optStep += optimizeZeroOneDivision(node);
    }
}

tree_t *differentiateEquation(tree_t *eq, char *var, const char *derivativeSymbol, int dumpLevel, FILE *dump) {
    assert(eq);

    tree_t *derivative = makeTree(nullptr);
    deleteNode(derivative->head);
    derivative->head = differentiateNode(eq->head, var, derivativeSymbol, dumpLevel, dump);

    optimizeSubequation(derivative->head);

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
    operation_t *op = makeOperation(VAR_CONST_PRIORITY, CONST, nullptr, nullptr, nullptr, nullptr,
                                    (double (*)(double, double)) nullptr, number);
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

    operation_t *var = makeOperation(VAR_CONST_PRIORITY, VAR, subst, nullptr, nullptr, nullptr,
                                     (double (*)(double, double)) nullptr, 0);
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
            op->exec.binary = EXEC_OPERATION; \
        } \
    }

#include "operations.h"

#undef DEF_FIRST_OP
#undef DEF_LAST_OP
#undef DEF_OP

}

node_t *getP(const char **e, char **vars) {
    assert(e);
    assert(*e);

    if (**e == '(') {
        nextChar(e);
        node_t *subtree = getE(e, vars);
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
            node_t *firstOp = getE(e, vars);
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
                node_t *secondOp = getE(e, vars);
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
        addVar(vars, ((operation_t *) subtree->value)->operation);
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

node_t *getT(const char **e, char **vars) {
    assert(e);
    assert(*e);

    node_t *subtree = getP(e, vars);
    node_t *suptree = nullptr;
    skipWhitespaces(e);
    while ((suptree = getFirstPriotityOp(**e))) {
        nextChar(e);
        node_t *subtree2 = getP(e, vars);
        mergeSubtrees(subtree, subtree2, suptree);
        subtree = suptree;
    }

    return subtree;
}

node_t *getE(const char **e, char **vars) {
    assert(e);
    assert(*e);

    node_t *subtree = getT(e, vars);
    node_t *suptree = nullptr;
    skipWhitespaces(e);
    while ((suptree = getLastPriotityOp(**e))) {
        nextChar(e);
        node_t *subtree2 = getT(e, vars);
        mergeSubtrees(subtree, subtree2, suptree);
        subtree = suptree;
    }
    return subtree;
}

tree_t *getG(const char *e, char **vars) {
    assert(e);
    node_t *subtree = getE(&e, vars);
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

