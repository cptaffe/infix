
/*
	CPSC2380
	Department of Computer Science, UALR
	Project 2
	Student Name: Connor Taffe
	Student UALR ID (last four digits): XXXX
	Project Description:
		This project takes input from stdin and lexes it into a thread-safe queue,
		items are popped off the queue by the parser, and lexed into a tree.
		The tree is then recursively evaluated by the parser and the result is
		displayed.
	Project Due Date: 9/9/2014
	Project Revised Date: 8/30/2014, 9/6/2014
*/


#include <thread>
#include "lex.h"
#include "parse.h"
#include "eval.h"

using namespace std;

int main(int argc, char *argv[]) {

	// init new lexer & parser
	lex *l = new lex(stdin);
	parse *p = new parse(l->getQue());

	// launch lexer & parser as threads
	thread lexer (&lex::lexLine, l);
	thread parser (&parse::parseLine, p);
	thread eval (&Evaluate, p->objs);

	// wait for them to finish
	lexer.join();
	parser.join();
	eval.join();

	// free allocated memory
	delete l;
	delete p;
}
