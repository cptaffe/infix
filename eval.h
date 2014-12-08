#ifndef INFIX_EVAL_H_
#define INFIX_EVAL_H_

#include <fcntl.h>
#include "obj.h"

// consumes trees from objs queue until it is done & empty,
// with each tree it prints the prefix, postfix, and infix notation.
// then it prints the numeric evaluation of each tree.
void Evaluate(que<Obj *> *objs) {
	Obj *o;
	while (!objs->isDone() || !objs->isEmpty()) {
		objs->pop(o);
		if (o == NULL) {continue;}
		if (o->isValid()) {
			flock(fileno(stdout), LOCK_EX);
			printf((char *) "prefix: %s\n", o->prefix()->c_str());
			printf("postfix: %s\n", o->postfix()->c_str());
			printf("infix: %s\n", o->infix()->c_str());
			printf("tree: %s\n", o->tree_print()->c_str());
			printf((char *) "=> %lld\n", o->eval());
			flock(fileno(stdout), LOCK_UN);
		}
		delete o;
	}
	delete objs;
}

#endif // INFIX_EVAL_H_
