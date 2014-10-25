#include "lex.cc"
#include "parse.cc"

int main(int argc, char *argv[]) {
	//if (argc < 2) {printf("need file name.\n"); exit(1);}
	//char **toks = (char **) calloc(sizeof(char *), 1024);
	//FILE *f = fopen(argv[1], "r");
	lex *l = new lex(stdin);
	char **toks = l->lex_all();
	parse *p = new parse(toks);
	p->parse_all();
	free(toks);
	//fclose(f);
	delete l;
	delete p;
}
