// DEF_OP(ID, ARGNUM, PRIORITY, LEXEME, LATEX_PREFIX, LATEX_INFIX, LATEX_SUFFIX, EXEC_OPERATION, DERIVATIVE)
DEF_FIRST_OP(BINARY, 0, "*", "", "\\cdot", "", *, 0);
DEF_FIRST_OP(BINARY, 0, "/", "\\fraq{", "}{", "}", /, 0);

DEF_SEC_OP(BINARY, 0, "+", "", "+", "", +, 0);
DEF__SEC_OP(BINARY, 0, "-", "", "-", "", -, 0);

DEF_OP(UNARY_FUNC, 0,  "cos", "\\cos{", "", "}", cos, 0);