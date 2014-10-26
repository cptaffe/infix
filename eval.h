#ifndef INFIX_EVAL_H_
#define INFIX_EVAL_H_

#import "obj.h"

// consumes trees from objs queue until it is done & empty,
// with each tree it prints the prefix, postfix, and infix notation.
// then it prints the numeric evaluation of each tree.
void Evaluate(que<Obj *> *objs) {
	Obj *o;
	while (!objs->isDone() || !objs->isEmpty()) {
		objs->pop(o);
		if (o == NULL) {continue;}
		if (o->isValid()) {
			cout << "prefix: " << *o->prefix() << endl;
			cout << "postfix: " << *o->postfix() << endl;
			cout << "infix: " << *o->infix() << endl;
			printf((char *) "=> %lld\n", o->eval());
		}
		delete o;
	}
	delete objs;
}

#endif // INFIX_EVAL_H_
