// DEF_OP(ID, ARGNUM, PRIORITY, LEXEME, LATEX_PREFIX, LATEX_INFIX, LATEX_SUFFIX, EXEC_OPERATION, DERIVATIVE)
DEF_FIRST_OP(1, BINARY, "*", "", " \\cdot ", "", nullptr, 0);
DEF_FIRST_OP(1, BINARY, "/", " \\frac{ ", " }{ ", " }", nullptr, 0);

DEF_LAST_OP(0, BINARY, "+", "", " + ", "", nullptr, 0);
DEF_LAST_OP(0, BINARY, "-", "", " - ", "", nullptr, 0);

DEF_OP(1, BINARY_FUNC, "pow", " { ", " }^{ ", " } ", nullptr, 0);
DEF_OP(1, UNARY_FUNC,  "cos", "\\cos{ ", "", " }", nullptr, 0);
DEF_OP(1, UNARY_FUNC,  "sin", "\\sin{ ", "", " }", nullptr, 0);