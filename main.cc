#include <thread>
#include "lex.h"
#include "parse.h"

using namespace std;

int main(int argc, char *argv[]) {
	//if (argc < 2) {printf("need file name.\n"); exit(1);}
	//char **toks = (char **) calloc(sizeof(char *), 1024);
	//FILE *f = fopen(argv[1], "r");
	char c;
	//printf("? "); fflush(stdout);
	//while ((c = getchar()) != EOF) {
		//ungetc(c, stdin);
		lex *l = new lex(stdin);
		parse *p = new parse(l->getQue());

		thread lexer (&lex::lexLine, l);
		thread parser (&parse::parseLine, p);

		lexer.join();
		parser.join();

		delete l;
		delete p;
		//printf("? "); fflush(stdout);
	//}
}
