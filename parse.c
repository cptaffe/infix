
// parse

#include <stdlib.h>
#include <stdint.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>

const size_t MMAPSIZE = 102400;

typedef enum {
	TYPE_OP,
	TYPE_NUM,
} Objtype;

struct Obj {
	Objtype type;
	union {
		int64_t num;
		char op;
	};
	struct Obj *rchild;
	struct Obj *lchild;
};

typedef struct Obj Obj;

Obj *init_Obj(int type) {
	Obj *o = calloc(sizeof(Obj), 1); // zeroes everything
	o->type = type;
	return o;
}

// recursive free
void free_Obj(Obj *o) {
	free_Obj(o->rchild);
	free_Obj(o->lchild);
	free(o);
}

typedef struct {
	char **toks;
	int toki;
	uint8_t *mem;
	int memi;
	Obj *root;
} parse;

parse *init_parse(char **toks) {
	parse *p = malloc(sizeof(parse));
	p->toks = toks;
	p->memi = 0; p->toki = 0;
	// mmap executable memory
	p->mem = mmap(NULL, MMAPSIZE, PROT_WRITE | PROT_EXEC, MAP_ANON | MAP_PRIVATE, -1, 0);
	p->root = NULL;
	return p;
}

void free_parse(parse *p) {
	munmap(p->mem, MMAPSIZE);
	free(p);
}

void push_ex(parse *p, char *mem, size_t len) {
	memcpy(&p->mem[p->memi], mem, len);
	p->memi += len;
}

static int getprec(char c) {
	if (c == '+' || c == '-') {return 0;}
	else if (c == '*' || c == '/') {return 1;}
	else if (c == '(') {return 2;}
	else {return -1;}
}

static void addchild(Obj **optr, Obj *no) {
	Obj *o = *optr;

	if (o == NULL) {
		*optr = no;
		return;
	}

	// printf("new type: %d, old type: %d\n", no->type, o->type);

	if (o->type == TYPE_OP) {
		if (no->type == TYPE_NUM) {
			// new tree is current tree's child
			// printf("new num is lchild of op.\n");
			if (o->lchild == NULL) {
				o->lchild = no; // add as child
			} else if (o->rchild == NULL){
				o->rchild = no; // add as child
			} else {
				addchild(&o->rchild, no);
			}
		} else {
			// compare op precedence
			int oprec = getprec(o->op);
			int nprec = getprec(no->op);
			if (oprec < nprec) {
				// recurse to position correctly
				if (o->rchild != NULL) {
					// printf("new op is rchild of old op (recursing).\n");
					addchild(&o->rchild, no);
				} else {
					// printf("new op is rchild of old op.\n");
					o->rchild = no;
				}
			} else {
				// printf("old op is rchild of new op.\n");
				*optr = no;
				no->rchild = o;
			}
		}
	} else if (no->type == TYPE_OP) {
		// current tree is new tree's child
		// printf("old op is rchild of new op.\n");
		*optr = no;
		no->lchild = o; // add as child
	} else {
		// printf("two nums, idk\n");
	}
	// error
}

// parens > * & / > + & -
static Obj *gentree(parse *p) {
	Obj *o = NULL;

	char *cur;
	while ((cur = p->toks[p->toki++]) != NULL) {
		if (isdigit(*cur)) {
			Obj *n = init_Obj(TYPE_NUM);
			n->num = atoi(cur); // num
			addchild(&o, n);
		} else {
			Obj *n = init_Obj(TYPE_OP);
			n->op = cur[0];
			addchild(&o, n);
		}
	}
	return o;
}

void printtree(Obj *o) {
	if (o != NULL) {
		if (o->type == TYPE_OP) {
			printtree(o->lchild);
			printf(" ");
			printtree(o->rchild);
			printf(" %c", o->op);
		} else {
			printf("%lld", o->num);
		}
	} else {
		printf("(nil)");
	}
}

// setup stack
void setup_stack(parse *p) {
	// init stack space
	push_ex(p, "\x55", 1); // push rbp
	push_ex(p, "\x48\x89\xe5", 3); // mov rbp to rsp
	push_ex(p, "\x48\x83\xec\x10", 4); // allocate 16 bytes of stack space
	push_ex(p, "\x56", 1); // push rsi
	push_ex(p, "\x57", 1); // push rdi
}

// reset stack
void reset_stack(parse *p) {
	// reset stack
	push_ex(p, "\x5f", 1); // pop rdi
	push_ex(p, "\x5e", 1); // pop rsi
	push_ex(p, "\x48\x89\xec", 3); // mov rsp to rbp
	push_ex(p, "\x5d", 1); // pop rbp
	push_ex(p, "\xc3", 1); // ret
}

void push_instr(parse *p, Obj *o) {
	switch (o->op) {
		case '+':
			push_ex(p, "\x48\x01\xd0", 3);
			break;
		case '-':
			push_ex(p, "\x48\x29\xd0", 3);
			break;
		case '*':
			push_ex(p, "\x48\xf7\xe2", 3);
			break;
		case '/':
			push_ex(p, "\x48\xba", 2); // rdx
			push_ex(p, (char *) "\x00\x00\x00\x00\x00\x00\x00\x00", 8); // 64 bit #
			push_ex(p, "\x48\xf7\xf1", 3); // divides rdx:rax by rcx
			break;
		default:
			printf("unknown op '%c'", o->op);
			return;
	}
}

void eval_tree(parse *p, Obj *o) {
	if (o != NULL) {
		if (o->type == TYPE_OP) {
			if (o->lchild->type != TYPE_NUM || o->rchild->type != TYPE_NUM) {
				if (o->lchild->type != TYPE_NUM) {eval_tree(p, o->lchild);}
				if (o->rchild->type != TYPE_NUM) {eval_tree(p, o->rchild);}
				if (o->op != '/') {
					if (o->lchild->type == TYPE_NUM) {
						push_ex(p, "\x48\xba", 2); // rdx
						push_ex(p, (char *) &o->lchild->num, 8); // 64 bit #
					}
					if (o->rchild->type == TYPE_NUM) {
						push_ex(p, "\x48\xba", 2); // rdx
						push_ex(p, (char *) &o->rchild->num, 8); // 64 bit #
					}
				} else {
					if (o->lchild->type == TYPE_NUM) {
						push_ex(p, "\x48\xb9", 2); // rdx
						push_ex(p, (char *) &o->lchild->num, 8); // 64 bit #
					}
					if (o->rchild->type == TYPE_NUM) {
						push_ex(p, "\x48\xb9", 2); // rdx
						push_ex(p, (char *) &o->rchild->num, 8); // 64 bit #
					}
				}
			} else {
				if (o->op != '/') {
					push_ex(p, "\x48\xb8", 2); // rax
					push_ex(p, (char *) &o->lchild->num, 8); // 64 bit #
					push_ex(p, "\x48\xba", 2); // rdx
					push_ex(p, (char *) &o->rchild->num, 8); // 64 bit #
				} else {
					push_ex(p, "\x48\xb8", 2); // rax
					push_ex(p, (char *) &o->lchild->num, 8); // 64 bit #
					push_ex(p, "\x48\xb9", 2); // rdx
					push_ex(p, (char *) &o->rchild->num, 8); // 64 bit #
				}
			}
			push_instr(p, o);
		} else {
			// push number to rdx
			push_ex(p, "\x48\xba", 2);
			push_ex(p, (char *) o->num, 8); // 64 bit #
		}
	} else {
		printf("(nil)");
	}
}

void parse_all(parse *p) {
	p->root = gentree(p);
	printtree(p->root); printf("\n");
	setup_stack(p);
	eval_tree(p, p->root);
	reset_stack(p);
	printf("result: %lld\n", ((int64_t (*)(int64_t)) p->mem)(8));
}

/*void eval(parse *p) {
	uint64_t i = 0;

	int64_t num = atoi(p->toks[i]); // get first #
	free(p->toks[i++]);

	// pop num to rax (accumulator)
	push_ex(p, "\x58", 1);

	printf("postfix: %lld ", num);
	while (p->toks[i] != NULL) {
		char op = p->toks[i][0];
		free(p->toks[i++]);

		int64_t num2 = atoi(p->toks[i]); // get #
		free(p->toks[i++]);

		// mov number to rdx
		push_ex(p, "\x48\xba", 2);
		push_ex(p, (char *) &num2, 8); // 64 bit #

		// get correct operationfrom op
		switch (op) {
		case '+':
			push_ex(p, "\x48\x01\xd0", 3);
			break;
		case '-':
			push_ex(p, "\x48\x29\xd0", 3);
			break;
		case '*':
			// TODO: mul
			break;
		case '/':
			// TODO: div
			break;
		default:
			printf("unknown op '%c'", op);
			return;
		}

		// stupid "print as postfix" thing...
		printf("%lld %c ", num2, op);
	}
	printf("\n");


	printf("result: %lld\n", ((int64_t (*)(int64_t)) p->mem)(num));
}*/
