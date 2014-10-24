#include "lex.c"
#include "parse.c"

int main() {
	char **toks = calloc(sizeof(char *), 1024);
	lex *l = init_lex(stdin, toks);
	if (l == NULL) {return 1;}
	parse *p = init_parse(toks);
	lex_all(l);
	parse_all(p);
	free_lex(l);
	free_parse(p);
	free(toks);
}
