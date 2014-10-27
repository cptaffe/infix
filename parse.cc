
// parser defs

#include <stdlib.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include "parse.h"

parse::parse(que<char *> *toks) {
	this->toks = toks;
	objs = new que<Obj *>();
	paren_mode = false;
	root = NULL;
}

parse::~parse() {
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
		#ifdef DEBUG
			flock(fileno(stdout), LOCK_EX);
			printf((char *) "recieved by parser: '%s'\n", tok);
			flock(fileno(stdout), LOCK_UN);
		#endif
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
