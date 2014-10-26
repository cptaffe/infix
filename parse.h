
// parse

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
	uint8_t *mem;
	int memi;
	bool paren_mode;
	Obj *root;
public:
	parse(que<char *> *);
	~parse();
	void push(char *, size_t);
	void addchild(Obj **, Obj *);
	void gentree(Obj **);
	void setup_stack();
	void reset_stack();
	void push_ex(char *, size_t);
	void eval_tree(Obj *);
	void parseLine();
};


parse::parse(que<char *> *toks) {
	this->toks = toks;
	memi = 0; paren_mode = false;
	// mmap executable memory
	mem = (uint8_t *) mmap(NULL, MMAPSIZE, PROT_WRITE | PROT_EXEC, MAP_ANON | MAP_PRIVATE, -1, 0);
	root = NULL;
}

parse::~parse() {
	munmap(mem, MMAPSIZE);
}

void parse::addchild(Obj **optr, Obj *no) {
	Obj *o = *optr;

	if (o == NULL) {
		if (no->type == TYPE_OP) {
			// printf((char *) "null replaced by new op (%c).\n", no->op);
		} else {
			// printf((char *) "null replaced by new num (%lld).\n", no->num);
		}
		*optr = no;
		return;
	}

	// printf("new type: %d, old type: %d\n", no->type, o->type);

	if (o->type == TYPE_OP) {
		if (no->type == TYPE_NUM) {
			// new tree is current tree's child
			// printf((char *) "new num (%lld) is lchild of op (%c).\n", no->num, o->op);
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
				} else if (o->lchild == NULL) {
					o->rchild = no;
				} else {
					addchild(&o->rchild, no);
				}
			} else {
				*optr = no;
				no->rchild = o;
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
void parse::gentree(Obj **tree) {
	char *tok;
	do {
		toks->pop(tok);
		if (tok == NULL) {break;}
		// printf("got: %s\n", tok);
		Obj *o = new Obj(tok);
		if (o->type == TYPE_OP && o->op == '(') {
			Obj *n = NULL;
			gentree(&n);
			n->prec = o->prec; // assign '(' precedence to this tree
			paren_mode = true;
			addchild(tree, n);
			paren_mode = false;
		} else if (o->type == TYPE_OP && o->op == ')') {
			return;
		} else {
			addchild(tree, o);
		}
	} while (tok != NULL);
}

void parse::parseLine() {
	while (!toks->isDone() || !toks->isEmpty()) {
		memi = paren_mode = 0; // reset
		gentree(&root);
		cout << "prefix: " << *root->postfix() << endl;
		cout << "postfix: " << *root->postfix() << endl;
		cout << "infix: " << *root->infix() << endl;
		printf((char *) "=> %lld\n", root->eval());
		delete root;
		root = NULL;
	}
}

#endif // INFIX_PARSE_H
