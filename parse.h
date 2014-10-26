
// parser

// the parser pops from the queue it shares with the lexer (toks).
// each string from that queue is parsed into an Obj and
// added to the tree. On the termination of each statement
// (symbolized by a null ptr on the queue), it pushes the tree
// to its queue and begins anew until the queue it shares with the
// lexer is done.

#ifndef INFIX_PARSE_H_
#define INFIX_PARSE_H_

#include <stdlib.h>
#include <stdint.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <iostream>
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


parse::parse(que<char *> *toks) {
	this->toks = toks;
	objs = new que<Obj *>();
	// memi = 0;
	paren_mode = false;
	// mmap executable memory
	//mem = (uint8_t *) mmap(NULL, MMAPSIZE, PROT_WRITE | PROT_EXEC, MAP_ANON | MAP_PRIVATE, -1, 0);
	root = NULL;
}

parse::~parse() {
	//munmap(mem, MMAPSIZE);
	delete toks;
}

void parse::addchild(Obj **optr, Obj *no) {
	Obj *o = *optr;

	if (o == NULL) {
		*optr = no;
	} else if (o->type == TYPE_OP) {
		if (no->type == TYPE_NUM) {
			// new tree is current tree's child
			if (o->lchild == NULL) {
				o->lchild = no; // add as child
			} else if (o->rchild == NULL){
				o->rchild = no; // add as child
			} else {
				addchild(&o->rchild, no);
			}
		} else {
			if (o->prec < no->prec) {
				// recurse to position correctly
				if (o->lchild == NULL) {
					o->lchild = no;
				} else if (o->rchild == NULL) {
					o->rchild = no;
				} else {
					addchild(&o->rchild, no);
				}
			} else {
				*optr = no;
				no->lchild = o;
			}
		}
	} else if (no->type == TYPE_OP) {
		// current tree is new tree's child
		*optr = no;
		no->lchild = o; // add as child
	}
	// error
}

// parens > * & / > + & -
bool parse::gentree(Obj **tree) {
	char *tok;
	do {
		if (toks->isDone() && toks->isEmpty()){return false;} // error
		toks->pop(tok);
		if (tok == NULL) {break;}
		Obj *o = new Obj(tok);
		if (o->type == TYPE_OP && o->op == '(') {
			Obj *n = NULL;
			gentree(&n);
			n->prec = o->prec; // assign '(' precedence to this tree
			paren_mode = true;
			addchild(tree, n);
			paren_mode = false;
		} else if (o->type == TYPE_OP && o->op == ')') {
			return true;
		} else {
			addchild(tree, o);
		}
		delete tok;
	} while (tok != NULL);
	return true; // worked
}

void parse::parseLine() {
	while (!toks->isDone() || !toks->isEmpty()) {
		// memi = 0;
		paren_mode = 0; // reset
		if (gentree(&root)){
			objs->push(root);
		}
		root = NULL;
	}
	objs->finish();
}

#endif // INFIX_PARSE_H
