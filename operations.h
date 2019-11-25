// DEF_OP(ID, ARGNUM, PRIORITY, LEXEME, LATEX_PREFIX, LATEX_INFIX, LATEX_SUFFIX, EXEC_OPERATION, DERIVATIVE)
DEF_FIRST_OP(BINARY, "*", "", "\\cdot", "", nullptr, 0);
DEF_FIRST_OP(BINARY, "/", "\\fraq{", "}{", "}", nullptr, 0);

DEF_LAST_OP(BINARY, "+", "", "+", "", nullptr, 0);
DEF_LAST_OP(BINARY, "-", "", "-", "", nullptr, 0);

DEF_OP(BINARY_FUNC, "pow", "{", "}^{", "}", nullptr, 0);
DEF_OP(UNARY_FUNC,  "cos", "\\cos{", "", "}", nullptr, 0);
DEF_OP(UNARY_FUNC,  "sin", "\\sin{", "", "}", nullptr, 0);