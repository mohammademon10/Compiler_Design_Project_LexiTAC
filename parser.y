%{
/*
 * parser.y — Bison Parser + Three-Address Code (TAC) Generator
 *
 * Grammar (simplified):
 *   stmt   → IDENTIFIER '=' expr
 *   expr   → expr '+' term | expr '-' term | term
 *   term   → term '*' factor | term '/' factor | factor
 *   factor → '(' expr ')' | NUMBER | IDENTIFIER
 *
 * Compile & Run:
 *   flex  lexer.l
 *   bison -d parser.y
 *   gcc   lex.yy.c parser.tab.c -o compiler -lfl
 *   echo "p = i + r * 60" | ./compiler
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ------------------------------------------------------------------ */
/*  Declarations from lexer.l                                          */
/* ------------------------------------------------------------------ */
extern int   yylex(void);
extern int   yylineno;
extern char *yytext;
void yyerror(const char *msg);

/* Symbol table — defined in lexer.l */
typedef struct { char name[64]; char id[8]; char type[8]; } Symbol;
extern Symbol symtab[];
extern int    symcount;
extern int    sym_index(const char *);
extern int    sym_add(const char *, const char *);
extern void   print_symtab(void);

/* ------------------------------------------------------------------ */
/*  TAC helpers                                                         */
/* ------------------------------------------------------------------ */
static int tmpcnt = 0;

static char *make_tmp(void) {
    char *t = malloc(8);
    snprintf(t, 8, "t%d", ++tmpcnt);
    return t;
}

static void emit(const char *result, const char *arg1,
                 const char *op,    const char *arg2) {
    if (op && arg2)
        printf("%s = %s %s %s\n", result, arg1, op, arg2);
    else
        printf("%s = %s\n",       result, arg1);
}
%}

/* ------------------------------------------------------------------ */
/*  Value type for semantic actions                                     */
/* ------------------------------------------------------------------ */
%union {
    char *str;   /* holds identifier id-string or literal value */
}

/* ------------------------------------------------------------------ */
/*  Token declarations                                                  */
/* ------------------------------------------------------------------ */
%token <str> IDENTIFIER NUMBER
%token ASSIGN PLUS MINUS MUL DIV LPAREN RPAREN NEWLINE

/* ------------------------------------------------------------------ */
/*  Non-terminal types                                                  */
/* ------------------------------------------------------------------ */
%type <str> expr term factor

/* ------------------------------------------------------------------ */
/*  Operator precedence (low → high)                                   */
/* ------------------------------------------------------------------ */
%left  PLUS MINUS
%left  MUL  DIV
%right UMINUS

%%
/* ================================================================== */
/*  Grammar rules                                                       */
/* ================================================================== */

program
    : /* empty */
    | program stmt
    ;

stmt
    : IDENTIFIER ASSIGN expr NEWLINE
        {
            /* Find the target variable id */
            int idx = sym_index($1);           /* $1 is already the id string  */
            /* Walk symtab to find by id */
            char lhs[8];
            strcpy(lhs, $1);                   /* $1 is id string from lexer   */
            emit(lhs, $3, NULL, NULL);
            free($1); free($3);
        }
    | error NEWLINE { yyerrok; }
    ;

expr
    : expr PLUS term
        {
            char *t = make_tmp();
            emit(t, $1, "+", $3);
            free($1); free($3);
            $$ = t;
        }
    | expr MINUS term
        {
            char *t = make_tmp();
            emit(t, $1, "-", $3);
            free($1); free($3);
            $$ = t;
        }
    | term  { $$ = $1; }
    ;

term
    : term MUL factor
        {
            char *t = make_tmp();
            emit(t, $1, "*", $3);
            free($1); free($3);
            $$ = t;
        }
    | term DIV factor
        {
            char *t = make_tmp();
            emit(t, $1, "/", $3);
            free($1); free($3);
            $$ = t;
        }
    | factor  { $$ = $1; }
    ;

factor
    : LPAREN expr RPAREN  { $$ = $2; }
    | NUMBER              { $$ = $1; }
    | IDENTIFIER          { $$ = $1; }
    | MINUS factor %prec UMINUS
        {
            char *t = make_tmp();
            emit(t, "-", $2, NULL);   /* unary minus as: t = - x */
            free($2);
            $$ = t;
        }
    ;

%%
/* ================================================================== */
/*  Supporting C code                                                   */
/* ================================================================== */

void yyerror(const char *msg) {
    fprintf(stderr, "\nParse Error (line %d): %s\n", yylineno, msg);
} 

int main(void) {
    char buffer[512];

    printf("=============================================================================\n");
    printf("         Flex + Bison: Symbol Table, Lexical Analysis & TAC Generator\n");
    printf("=============================================================================\n");
    printf("Enter expression(s). One per line. Ctrl-D to finish.\n");
    printf("Example: p = i + r * 60\n\n");

    /* ---- Phase 1 header ---- */
    printf("--- Lexical Tokens ---\n");

    yyparse();          /* drives yylex(); TAC emitted inside rules */

    /* ---- Symbol table (populated during lex) ---- */
    print_symtab();

    printf("\n(TAC emitted above during parsing.)\n");
    printf("=============================================================================\n");
    return 0;
}
