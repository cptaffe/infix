
// lexer program

#ifndef INFIX_LEX_H_
#define INFIX_LEX_H_

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <ctype.h>
#include <string.h>
#include <iostream>
#include "que.h" // locking que.

using namespace std;

const size_t BUFSIZE = 256; // 0.25kb

class lex {
	FILE *code;
	char *buf;
	que<char *> *toks;
	int i; // index of buf
public:
	lex(FILE *f);
	~lex();
	char next();
	void back();
	char peek();
	void emit();
	int len();
	void dump();
	void killspace();
	void lexLine();
	void bufresize();
	que<char *> *getQue();
};

typedef void *(*lexfunc)(lex *);

lex::lex(FILE *f) {
	code = f;
	buf = (char *) calloc(sizeof(char), BUFSIZE);
	toks = new que<char *>();
	i = 0;
}

lex::~lex() {
	//fclose(l->code);
	delete buf;
}

que<char *> *lex::getQue() {
	return toks;
}


char lex::next() {
	buf[i] = getc(code);
	if (buf[i] == EOF) {return EOF;}
	i++;
	return buf[i - 1];
}

void lex::back() {
	i--;
	ungetc(buf[i], code);
}

char lex::peek() {
	char c = next();
	back();
	return c;
}

// interacts with mutexed ring buffer,
// sending tokens (substrings),
// so it runs only when needed.
void lex::emit() {
	char *str = (char *) malloc(i + 1);
	memcpy(str, buf, i);
	str[i] = '\0';
	// printf("emit: %s\n", str);
	toks->push(str);
	i = 0; // reset
}

int lex::len() {
	return i;
}

void lex::dump() {
	i = 0;
}

void lex::killspace() {
	char c = next();
	while (isspace(c) && c != '\n') {c = next();} // lex whitespace
	back(); dump(); // stupid whitespace
}

void *lex_num(lex *l);

// lex_op
void *lex_op(lex *l) {
	l->killspace();
	char ops[] = "+-*/x÷";
	while(memchr(&ops, l->next(), sizeof(ops)) != NULL) {}
	l->back();
	if (l->len() > 0) {
		l->emit();
		return (void *) lex_num;
	} else if (l->peek() == ')') {
		l->next(); l->emit(); // emit paren
		return (void *) lex_op;
	} else if (l->peek() == EOF || l->peek() == '\n') {
		l->next(); // lex
		l->dump(); // ignore
		return NULL;
	} else {
		fprintf(stderr, "expected operator, not '%c'.\n", l->peek());
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
		l->next(); // lex
		l->dump(); // ignore
		return NULL;
	} else {
		fprintf(stderr, "expected number, not '%c'.\n", l->peek());
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
}

#endif // INFIX_LEX_H_
