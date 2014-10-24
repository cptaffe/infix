
// lexer program

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <ctype.h>
#include <string.h>

const size_t BUFSIZE = 256; // 0.25kb

typedef struct {
	FILE *code;
	char *buf;
	char **toks;
	int toki;
	int i; // index of buf
} lex;

typedef void *(*lexfunc)(lex *);

lex *init_lex(FILE *f, char **toks) {
	lex *l = malloc(sizeof(lex));
	l->code = f;
	if (l->code == NULL) {return NULL;}
	l->buf = calloc(sizeof(char), BUFSIZE);
	l->toks = toks;
	l->i = 0;
	l->toki = 0;
	return l;
}

void free_lex(lex *l) {
	fclose(l->code);
	free(l->buf);
	free(l);
}

static char next(lex *l) {
	l->buf[l->i] = getc(l->code);
	if (l->buf[l->i] == EOF) {return EOF;}
	l->i++;
	return l->buf[l->i - 1];
}

static void back(lex *l) {
	l->i--;
	ungetc(l->buf[l->i], l->code);
}

static char peek(lex *l) {
	char c = next(l);
	back(l);
	return c;
}

// interacts with mutexed ring buffer,
// sending tokens (substrings),
// so it runs only when needed.
static void emit(lex *l) {
	char *str = malloc(l->i + 1);
	memcpy(str, l->buf, l->i);
	str[l->i] = '\0';
	l->toks[l->toki] = str;
	// printf("emit: '%s'\n", l->toks[l->toki]);
	l->toki++;
	l->i = 0; // reset
}

static int len(lex *l) {
	return l->i;
}

static void dump(lex *l) {
	l->i = 0;
}

static void killspace(lex *l) {
	char c = next(l);
	while (isspace(c) && c != '\n') {c = next(l);} // lex whitespace
	back(l); dump(l); // stupid whitespace
}

void *lex_num(lex *l);

// lex_op
void *lex_op(lex *l) {
	killspace(l);
	char ops[] = "+-*/";
	while(memchr(&ops, next(l), sizeof(ops)) != NULL) {}
	back(l);
	if (len(l) > 0) {
		emit(l);
		return lex_num;
	} else if (peek(l) == ')') {
		next(l); emit(l); // emit paren
		return lex_op;
	} else if (peek(l) == EOF || peek(l) == '\n') {
		dump(l); // ignore
		return NULL;
	} else {
		fprintf(stderr, "expected operator, not '%c'.\n", peek(l));
		return NULL;
	}
}

// lex_all
void *lex_num(lex *l) {
	killspace(l);
	while (isdigit(next(l))) {}
	back(l);
	if (len(l) > 0) {
		emit(l);
		return lex_op;
	} else if (peek(l) == '(') {
		next(l); emit(l); // emit paren
		return lex_num;
	} else if (peek(l) == EOF || peek(l) == '\n') {
		dump(l); // ignore
		return NULL;
	} else {
		fprintf(stderr, "expected number, not '%c'.\n", peek(l));
		return NULL;
	}
}

// finite state machine
void lex_all(lex *l) {
	lexfunc lexer = lex_num;
	while (lexer != NULL) {
		lexer = (lexfunc) lexer(l);
	}
}
