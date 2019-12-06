// DEF_OP(ID, ARGNUM, PRIORITY, LEXEME, LATEX_PREFIX, LATEX_INFIX, LATEX_SUFFIX, EXEC_OPERATION, DERIVATIVE)
DEF_FIRST_OP(1, BINARY, "*", "", " \\cdot ", "", (double (*)(double, double))([](double a, double b) -> double {return a * b;}), ALWAYS(DFUNC("+", DFUNC("*", dL, R), DFUNC("*", dR, L))));
DEF_FIRST_OP(1, BINARY, "/", " \\frac{ ", " }{ ", " }", (double (*)(double, double))([](double a, double b) -> double {return a / b;}), ALWAYS(DFUNC("/", DFUNC("-", DFUNC("*", dL, R), DFUNC("*", dR, L)), DFUNC("pow", R, CONST(2)))));

DEF_LAST_OP(0, BINARY, "+", "", " + ", "", (double (*) (double, double))([](double a, double b) -> double {return a + b;}), ALWAYS(DFUNC("+", dL, dR)));
DEF_LAST_OP(0, BINARY, "-", "", " - ", "", (double (*) (double, double))([](double a, double b) -> double {return a - b;}), ALWAYS(DFUNC("-", dL, dR)));


DEF_OP(1, UNARY_FUNC, "ln", "\\ln{", "", "}", (double (*) (double, double))log, ALWAYS(DFUNC("/", CONST(1), R)));
DEF_OP(1, BINARY_FUNC, "pow", " { ", " }^{ ", " } ", (double (*) (double, double))pow, SPECIAL_CASE(WHATEVER, CONSTANT, DFUNC("*", DFUNC("pow", L, DFUNC("-", R, CONST(1))), R)) SPECIAL_CASE(WHATEVER, PSEUDOCONSTANT, DFUNC("*", DFUNC("pow", L, DFUNC("-", R, CONST(1))), R)) ALWAYS(DFUNC("*", DFUNC("pow", L, R), DFUNC("+", DFUNC("*", dR, FUNC("ln", L)), DFUNC("/", DFUNC("*", R, dL), L)))));
DEF_OP(1, UNARY_FUNC,  "cos", "\\cos{ ", "", " }", (double (*) (double, double))cos, ALWAYS(DFUNC("*", DFUNC("*", CONST(-1), dR), FUNC("sin", R))));
DEF_OP(1, UNARY_FUNC,  "sin", "\\sin{ ", "", " }", (double (*) (double, double))sin, ALWAYS(DFUNC("*", dR, FUNC("cos", R))));