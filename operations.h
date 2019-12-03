// DEF_OP(ID, ARGNUM, PRIORITY, LEXEME, LATEX_PREFIX, LATEX_INFIX, LATEX_SUFFIX, EXEC_OPERATION, DERIVATIVE)
DEF_FIRST_OP(1, BINARY, "*", "", " \\cdot ", "", (void *)(double (*)(double, double))([](double a, double b) -> double {return a * b;}), DFUNC("+", DFUNC("*", dL, R), DFUNC("*", dR, L)));
DEF_FIRST_OP(1, BINARY, "/", " \\frac{ ", " }{ ", " }",(void *)(double (*)(double, double))([](double a, double b) -> double {return a / b;}), DFUNC("/", DFUNC("-", DFUNC("*", dL, R), DFUNC("*", dR, L)), DFUNC("pow", R, CONST(2))));

DEF_LAST_OP(0, BINARY, "+", "", " + ", "", (void *)(double (*) (double, double))([](double a, double b) -> double {return a + b;}), DFUNC("+", dL, dR));
DEF_LAST_OP(0, BINARY, "-", "", " - ", "", (void *)(double (*) (double, double))([](double a, double b) -> double {return a + b;}), DFUNC("-", dL, dR));

DEF_OP(1, BINARY_FUNC, "pow", " { ", " }^{ ", " } ", (void *)pow, 0);
DEF_OP(1, UNARY_FUNC,  "cos", "\\cos{ ", "", " }", (void *)cos, DFUNC("*", DFUNC("*", CONST(-1), dR), FUNC("sin", R)));
DEF_OP(1, UNARY_FUNC,  "sin", "\\sin{ ", "", " }", (void *)sin, DFUNC("*", dR, FUNC("cos", R)));