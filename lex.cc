
// lexer defs

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <stdarg.h>
#include <fcntl.h>
#include "lex.h"

lex::lex(FILE *f) {
	code = f;
	buf = (char *) calloc(sizeof(char), BUFSIZE);
	toks = new que<char *>();
	i = 0;
}

lex::~lex() {
	delete buf;
}

// returns lexer's queue.
que<char *> *lex::getQue() {
	return toks;
}

void lex::err(char *fmt, ...) {
	va_list ap;
	va_start(ap, fmt);
	flock(fileno(stderr), LOCK_EX);
	vfprintf(stderr, fmt, ap);
	flock(fileno(stderr), LOCK_UN);
	va_end(ap);
}

// advances one character.
char lex::next() {
	buf[i] = getc(code);
	if (buf[i] == EOF) {return EOF;}
	i++;
	return buf[i - 1];
}

// backs up one character.
void lex::back() {
	i--;
	ungetc(buf[i], code);
}

// checks what next character is without advancing.
char lex::peek() {
	char c = next();
	back();
	return c;
}

// emit sends data (tokens) as strings over the queue.
void lex::emit() {
	char *str = (char *) malloc(i + 1);
	memcpy(str, buf, i);
	str[i] = '\0';
	#ifdef DEBUG
		flock(fileno(stdout), LOCK_EX);
		printf("emitted by lex: '%s'\n", str);
		flock(fileno(stdout), LOCK_UN);
	#endif
	toks->push(str);
	i = 0; // reset
}

// gets the length of the currently lexed string.
int lex::len() {
	return i;
}

// ignores the currently lexed string.
void lex::dump() {
	i = 0;
}

void lex::killspace() {
	char c = next();
	while (isspace(c) && c != '\n') {c = next();} // lex whitespace
	back(); dump(); // stupid whitespace
}

void lex::killline() {
	while (next() != '\n') {} // lex whitespace
	dump();
}

void *lex_num(lex *l);

// lex_op
void *lex_op(lex *l) {
	l->killspace();
	char ops[] = "+-*/";
	while(memchr(&ops, l->next(), sizeof(ops)) != NULL) {}
	l->back();
	if (l->len() > 0) {
		l->emit();
		return (void *) lex_num;
	} else if (l->peek() == ')') {
		l->next(); l->emit(); // emit paren
		return (void *) lex_op;
	} else if (l->peek() == EOF || l->peek() == '\n') {
		l->killline();
		return NULL;
	} else {
		l->err((char *) "err: expected operator, not '%c'.\n", l->peek());
		l->killline();
		return NULL;
	}
}

// lex_all
void *lex_num(lex *l) {
	l->killspace();
	while (isdigit(l->next())) {}
	l->back();
	if (l->len() > 0) {
		l->emit();
		return (void *) lex_op;
	} else if (l->peek() == '(') {
		l->next(); l->emit(); // emit paren
		return (void *) lex_num;
	} else if (l->peek() == EOF || l->peek() == '\n') {
		l->err((char *) "err: expected number, not newline.\n");
		l->killline();
		return NULL;
	} else {
		l->err((char *) "err: expected number, not '%c'.\n", l->peek());
		l->killline();
		return NULL;
	}
}

// finite state machine
void lex::lexLine() {
	while (peek() != EOF) {
		lexfunc lexer = lex_num;
		while (lexer != NULL) {
			lexer = (lexfunc) lexer(this);
		}
		toks->push(NULL); // end of statement
	}
	toks->finish();
}
