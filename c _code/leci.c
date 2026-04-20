#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

// --- Windows Unicode Fix ---
#ifdef _WIN32
#include <windows.h>
#endif

// --- ANSI Color Codes ---
#define CYN   "\x1B[36m"
#define YEL   "\x1B[33m"
#define GRN   "\x1B[32m"
#define RED   "\x1B[31m"
#define MAG   "\x1B[35m"
#define BOLD  "\x1B[1m"
#define RESET "\x1B[0m"

// Renamed to LexTokenType to avoid conflict with windows.h
typedef enum {
    T_IDENTIFIER, T_NUMBER, T_PLUS, T_MINUS, T_MUL, T_DIV,
    T_LPAREN, T_RPAREN, T_ASSIGN, T_END, T_INVALID
} LexTokenType;

typedef struct {
    LexTokenType type;
    char lexeme[64];
} Token;

typedef struct {
    char name[64];
    char id[8];
} Symbol;

/* --- Function Prototypes --- */
Token next_token();
void generate_tac(char *target, char *arg1, char *op, char *arg2);
char* parse_expr();
char* parse_term();
char* parse_factor();
void advance();

Symbol symtab[256];
int symcount = 0;
char *src;
int src_pos = 0;
int tmpcnt = 0;

char temp_names[100][10];
int temp_index = 0;
char constant_buffer[64];
Token current_token;

void advance() {
    current_token = next_token();
}

char* parse_expr();
char* parse_term();
char* parse_factor();

/* --- Utility Functions --- */
// Adds a symbol to the symbol table if it doesn't exist, returns index
int sym_add(const char *name) {
    for (int i = 0; i < symcount; i++) {
        if (strcmp(symtab[i].name, name) == 0) return i;
    }
    strcpy(symtab[symcount].name, name);
    snprintf(symtab[symcount].id, 8, "id%d", symcount + 1);
    return symcount++;
}
 // Lexical analyzer: returns the next token from the source code
Token next_token() {
    Token t; t.type = T_INVALID;
    memset(t.lexeme, 0, 64);
    
    while (src[src_pos] && isspace(src[src_pos])) src_pos++;
    if (!src[src_pos]) { t.type = T_END; return t; }

    char c = src[src_pos];
    if (isalpha(c)) {
        int i = 0;
        while (isalnum(src[src_pos])) t.lexeme[i++] = src[src_pos++];
        t.lexeme[i] = '\0'; t.type = T_IDENTIFIER;
    } else if (isdigit(c)) {
        int i = 0;
        while (isdigit(src[src_pos])) t.lexeme[i++] = src[src_pos++];
        t.lexeme[i] = '\0'; t.type = T_NUMBER;
    } else {
        t.lexeme[0] = src[src_pos++]; t.lexeme[1] = '\0';
        if (c == '+') t.type = T_PLUS; else if (c == '-') t.type = T_MINUS;
        else if (c == '*') t.type = T_MUL; else if (c == '/') t.type = T_DIV;
        else if (c == '(') t.type = T_LPAREN; else if (c == ')') t.type = T_RPAREN;
        else if (c == '=') t.type = T_ASSIGN;
    }
    return t;
}

/* --- Parsing Functions --- */
// Parses factors: identifiers, numbers, or parenthesized expressions
char* parse_factor() {
    if (current_token.type == T_IDENTIFIER) {
        int idx = sym_add(current_token.lexeme);
        advance();
        return symtab[idx].id;
    } else if (current_token.type == T_NUMBER) {
        strcpy(constant_buffer, current_token.lexeme);
        advance();
        return constant_buffer;
    } else if (current_token.type == T_LPAREN) {
        advance();
        char* res = parse_expr();
        if (current_token.type != T_RPAREN) {
            printf(RED "Error: Missing ')'\n" RESET);
            exit(1);
        }
        advance();
        return res;
    } else {
        printf(RED "Error: Unexpected token in factor\n" RESET);
        exit(1);
    }
}

// Parses terms: factors combined with '*' or '/'
char* parse_term() {
    char* result = parse_factor();
    while (current_token.type == T_MUL || current_token.type == T_DIV) {
        char* op = (current_token.type == T_MUL) ? "*" : "/";
        advance();
        char* right = parse_factor();
        sprintf(temp_names[temp_index], "t%d", ++tmpcnt);
        generate_tac(temp_names[temp_index], result, op, right);
        result = temp_names[temp_index++];
    }
    return result;
}

// Parses expressions: terms combined with '+' or '-' 
char* parse_expr() {
    char* result = parse_term();
    while (current_token.type == T_PLUS || current_token.type == T_MINUS) {
        char* op = (current_token.type == T_PLUS) ? "+" : "-";
        advance();
        char* right = parse_term();
        sprintf(temp_names[temp_index], "t%d", ++tmpcnt);
        generate_tac(temp_names[temp_index], result, op, right);
        result = temp_names[temp_index++];
    }
    return result;
}

/* --- Improved UI Printing --- */
// Prints a stylized banner at the start of the program
void print_banner() {
    printf(CYN BOLD "┌──────────────────────────────────────────────────────────┐\n");
    printf("│  SYMBOL TABLE, LEXICAL ANALYSIS & TAC GENERATOR          │\n");
    printf("└──────────────────────────────────────────────────────────┘\n" RESET);
}

void print_symbol_table() {
    printf(YEL "\n[1] SYMBOL TABLE\n" RESET);
    printf("┌────────────────┬────────────────┐\n");
    printf("│  " BOLD "%-12s" RESET "  │  " BOLD "%-12s" RESET " │\n", "Identifier", "Internal ID");
    printf("├────────────────┼────────────────┤\n");
    for (int i = 0; i < symcount; i++) {
        printf("│ %-14s │ %-14s │\n", symtab[i].name, symtab[i].id);
    }
    printf("└────────────────┴────────────────┘\n");
}

void generate_tac(char *target, char *arg1, char *op, char *arg2) {
    if (arg2)
        printf(GRN "    %-5s" RESET " = " MAG "%s %s %s" RESET "\n", target, arg1, op, arg2);
    else
        printf(GRN "    %-5s" RESET " = " MAG "%s" RESET "\n", target, arg1);
}

int main() {
    #ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
    #endif

    char buffer[256];
    print_banner();
    printf(BOLD "Enter Expression: " RESET);
    if (!fgets(buffer, 256, stdin)) return 0;
    src = buffer;

    printf("\n[*] Initializing Compiler Phases...\n");
    
    src_pos = 0;
    Token t;
    while ((t = next_token()).type != T_END) {
        if (t.type == T_IDENTIFIER) {
            sym_add(t.lexeme);
        }
    }

    print_symbol_table();

    printf(YEL "\n[2] THREE ADDRESS CODE (TAC)\n" RESET);
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

    printf(CYN "\n[*] Process Complete.\n" RESET);
    return 0;
}