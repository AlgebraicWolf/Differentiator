// DEF_OP(ARGNUM, PRIORITY, LEXEME, LATEX_PREFIX, LATEX_INFIX, LATEX_SUFFIX, EXEC_OPERATION, DERIVATIVE)
DEF_OP(2, 0, "*", "", "\\cdot", "", *, 0);
DEF_OP(2, 0, "/", "\\fraq{", "}{", "}", /, 0);
DEF_OP(2, 0, "+", "", "+", "", +, 0);
DEF_OP(2, 0, "-", "", "-", "", -, 0);
DEF_OP(1, 0,  "cos", "\\cos{", "", "}", cos, 0);