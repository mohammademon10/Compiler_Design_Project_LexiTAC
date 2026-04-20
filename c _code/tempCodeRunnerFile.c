// Parse and generate TAC
    src_pos = 0; // reset position
    advance();
    if (current_token.type != T_IDENTIFIER) {
        printf(RED "Error: Expected identifier for assignment\n" RESET);
        return 1;
    }
    int idx = sym_add(current_token.lexeme);
    char* target = symtab[idx].id;
    advance();
    if (current_token.type != T_ASSIGN) {
        printf(RED "Error: Expected '='\n" RESET);
        return 1;
    }
    advance();
    char* expr_result = parse_expr();
    if (current_token.type != T_END) {
        printf(RED "Error: Unexpected tokens after expression\n" RESET);
        return 1;
    }
    generate_tac(target, expr_result, NULL, NULL);