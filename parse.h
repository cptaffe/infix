
// parser

// the parser pops from the queue it shares with the lexer (toks).
// each string from that queue is parsed into an Obj and
// added to the tree. On the termination of each statement
// (symbolized by a null ptr on the queue), it pushes the tree
// to its queue and begins anew until the queue it shares with the
// lexer is done.

#ifndef INFIX_PARSE_H_
#define INFIX_PARSE_H_

#include "que.h"
#include "obj.h"

using namespace std;

const size_t MMAPSIZE = 102400;

class parse {
	que<char *> *toks;
	// uint8_t *mem;
	// int memi;
	bool paren_mode;
	Obj *root;
public:
	que<Obj *> *objs;
	parse(que<char *> *);
	~parse();
	void addchild(Obj **, Obj *);
	bool gentree(Obj **);
	void parseLine();
};

#endif // INFIX_PARSE_H
