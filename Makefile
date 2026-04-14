# =============================================================================
# Makefile — Flex + Bison Compiler Project
# =============================================================================
# Requirements:
#   sudo apt install flex bison gcc    (Debian/Ubuntu)
#   brew install flex bison gcc        (macOS via Homebrew)
#
# Usage:
#   make            → build ./compiler
#   make run        → run with example input
#   make clean      → remove generated files
# =============================================================================

CC      = gcc
CFLAGS  = -Wall -Wextra -g
TARGET  = compiler

# Generated source files
LEX_SRC = lex.yy.c
PAR_SRC = parser.tab.c
PAR_HDR = parser.tab.h

.PHONY: all run clean

all: $(TARGET)

## Step 1 — Bison generates parser.tab.c + parser.tab.h
$(PAR_SRC) $(PAR_HDR): parser.y
	bison -d parser.y

## Step 2 — Flex generates lex.yy.c  (needs parser.tab.h for token codes)
$(LEX_SRC): lexer.l $(PAR_HDR)
	flex lexer.l

## Step 3 — Compile everything together
$(TARGET): $(LEX_SRC) $(PAR_SRC)
	$(CC) $(CFLAGS) -o $@ $(LEX_SRC) $(PAR_SRC) -lfl

## Run a quick test
run: $(TARGET)
	@echo "p = i + r * 60" | ./$(TARGET)

## Remove all generated files
clean:
	rm -f $(LEX_SRC) $(PAR_SRC) $(PAR_HDR) $(TARGET)
