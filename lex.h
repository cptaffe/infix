
// lexer program

#ifndef INFIX_LEX_H_
#define INFIX_LEX_H_

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
	void err(char *fmt, ...);
	int len();
	void dump();
	void killspace();
	void killline();
	void lexLine();
	void bufresize();
	que<char *> *getQue();
};

typedef void *(*lexfunc)(lex *);

#endif // INFIX_LEX_H_
