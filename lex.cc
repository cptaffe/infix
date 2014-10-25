
// lexer program

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <ctype.h>
#include <string.h>

const size_t BUFSIZE = 256; // 0.25kb

class lex {
	FILE *code;
	char *buf;
	char **toks;
	int buflen;
	int toki;
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
	void reset();
	void killspace();
	char **lex_all();
	void bufresize();
};

typedef void *(*lexfunc)(lex *);

lex::lex(FILE *f) {
	code = f;
	buf = (char *) calloc(sizeof(char), BUFSIZE);
	buflen = 1024;
	toks = (char **) calloc(sizeof(char *), buflen);
	i = 0;
	toki = 0;
}

void lex::bufresize() {
	buflen *= 2; // double
	toks = (char **) realloc(toks, sizeof(char *) * buflen);
}

lex::~lex() {
	//fclose(l->code);
	delete buf;
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
	toks[toki] = str;
	//printf((char *) "emit: '%s'\n", toks[toki]);
	toki++;
	if (toki >= buflen) {bufresize();}
	i = 0; // reset
}

int lex::len() {
	return i;
}

void lex::dump() {
	i = 0;
}

void lex::reset() {
	toki = i = 0;
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
		l->dump(); // ignore
		return NULL;
	} else {
		fprintf(stderr, "expected number, not '%c'.\n", l->peek());
		return NULL;
	}
}

// finite state machine
char **lex::lex_all() {
	lexfunc lexer = lex_num;
	while (lexer != NULL) {
		lexer = (lexfunc) lexer(this);
	}
	return toks;
	//reset(); // reset
}
