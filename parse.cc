
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

class Obj;

class parse {
	char **toks;
	int toki;
	uint8_t *mem;
	int memi;
	int paren_mode;
	Obj *root;
public:
	parse(char **toks);
	~parse();
	void push(char *, size_t);
	int getprec(char);
	void addchild(Obj **, Obj *);
	Obj *gentree();
	void setup_stack();
	void reset_stack();
	void push_ex(char *, size_t);
	void eval_tree(Obj *);
	void parse_all();
};

class Obj {
	friend class parse;
	Objtype type;
	union {
		int64_t num;
		char op;
	};
	Obj *rchild;
	Obj *lchild;
public:
	Obj(int);
	~Obj();
	void print();
	void push_instr(parse *p);
};

Obj::Obj(int type) {
	num = 0;
	rchild = NULL; lchild = NULL;
	this->type = (Objtype) type;
}

// recursive free
Obj::~Obj() {
	if (rchild != NULL) {delete rchild;}
	if (lchild != NULL) {delete lchild;}
}

parse::parse(char **toks) {
	this->toks = toks;
	memi = 0; toki = 0; paren_mode = 0;
	// mmap executable memory
	mem = (uint8_t *) mmap(NULL, MMAPSIZE, PROT_WRITE | PROT_EXEC, MAP_ANON | MAP_PRIVATE, -1, 0);
	root = NULL;
}

parse::~parse() {
	munmap(mem, MMAPSIZE);
}

void parse::push_ex(char *m, size_t len) {
	memcpy(&mem[memi], m, len);
	memi += len;
}

int parse::getprec(char c) {
	if (c == '+' || c == '-') {return 0;}
	else if (c == '*' || c == '/') {return 1;}
	else if (c == '(') {return 2;}
	else {return -1;}
}

void parse::addchild(Obj **optr, Obj *no) {
	Obj *o = *optr;

	if (o == NULL) {
		*optr = no;
		return;
	}

	// printf("new type: %d, old type: %d\n", no->type, o->type);

	if (o->type == TYPE_OP) {
		if (no->type == TYPE_NUM) {
			// new tree is current tree's child
			// printf((char *) "new num is lchild of op.\n");
			if (o->lchild == NULL) {
				o->lchild = no; // add as child
			} else if (o->rchild == NULL){
				o->rchild = no; // add as child
			} else {
				addchild(&o->rchild, no);
			}
		} else {
			if (no->op == '(') {paren_mode = 1; return;}
			else if (no->op == ')') {paren_mode = 0; return;}
			// compare op precedence
			int oprec = getprec(o->op);
			int nprec;
			if (!paren_mode) {
				nprec = getprec(no->op);
			} else {
				nprec = 10; // supa max
			}

			if (oprec < nprec) {
				// recurse to position correctly
				if (o->rchild != NULL) {
					// printf((char *) "new op is rchild of old op (recursing).\n");
					addchild(&o->rchild, no);
				} else {
					// printf((char *) "new op is rchild of old op.\n");
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
		// printf((char *) "old op is rchild of new op.\n");
		*optr = no;
		no->lchild = o; // add as child
	} else {
		// printf((char *) "two nums, idk\n");
	}
	// error
}

// parens > * & / > + & -
Obj *parse::gentree() {
	Obj *o = NULL;

	char *cur;
	while ((cur = toks[toki++]) != NULL) {
		if (isdigit(*cur)) {
			Obj *n = new Obj(TYPE_NUM);
			n->num = atoi(cur); // num
			addchild(&o, n);
		} else {
			Obj *n = new Obj(TYPE_OP);
			n->op = cur[0];
			addchild(&o, n);
		}
	}
	return o;
}

void Obj::print() {
	if (this != NULL) {
		if (type == TYPE_OP) {
			lchild->print();
			printf((char *)" ");
			rchild->print();
			printf((char *)" %c", op);
		} else {
			printf((char *)"%lld", num);
		}
	} else {
		printf("(nil)");
	}
}

// setup stack
void parse::setup_stack() {
	// init stack space
	push_ex((char *) "\x55", 1); // push rbp
	push_ex((char *) "\x48\x89\xe5", 3); // mov rbp to rsp
	push_ex((char *) "\x48\x83\xec\x10", 4); // allocate 16 bytes of stack space
	push_ex((char *) "\x56", 1); // push rsi
	push_ex((char *) "\x57", 1); // push rdi
}

// reset stack
void parse::reset_stack() {
	// reset stack
	push_ex((char *) "\x5f", 1); // pop rdi
	push_ex((char *) "\x5e", 1); // pop rsi
	push_ex((char *) "\x48\x89\xec", 3); // mov rsp to rbp
	push_ex((char *) "\x5d", 1); // pop rbp
	push_ex((char *) "\xc3", 1); // ret
}

void Obj::push_instr(parse *p) {
	switch (op) {
		case '+':
			p->push_ex((char *) "\x48\x01\xd0", 3);
			break;
		case '-':
			p->push_ex((char *) "\x48\x29\xd0", 3);
			break;
		case '*':
			p->push_ex((char *) "\x48\xf7\xe2", 3);
			break;
		case '/':
			p->push_ex((char *) "\x48\xba", 2); // rdx
			p->push_ex((char *) "\x00\x00\x00\x00\x00\x00\x00\x00", 8); // 64 bit #
			p->push_ex((char *) "\x48\xf7\xf1", 3); // divides rdx:rax by rcx
			break;
		default:
			printf((char *) "unknown op '%c'", op);
			return;
	}
}

void parse::eval_tree(Obj *o) {
	if (o != NULL) {
		if (o->type == TYPE_OP) {
			if (o->lchild->type != TYPE_NUM || o->rchild->type != TYPE_NUM) {
				if (o->lchild->type != TYPE_NUM) {eval_tree(o->lchild);}
				if (o->rchild->type != TYPE_NUM) {eval_tree(o->rchild);}
				if (o->op != '/') {
					if (o->lchild->type == TYPE_NUM) {
						push_ex((char *) "\x48\xba", 2); // rdx
						push_ex((char *) &o->lchild->num, 8); // 64 bit #
					}
					if (o->rchild->type == TYPE_NUM) {
						push_ex((char *) "\x48\xba", 2); // rdx
						push_ex((char *) &o->rchild->num, 8); // 64 bit #
					}
				} else {
					if (o->lchild->type == TYPE_NUM) {
						push_ex((char *) "\x48\xb9", 2); // rdx
						push_ex((char *) &o->lchild->num, 8); // 64 bit #
					}
					if (o->rchild->type == TYPE_NUM) {
						push_ex((char *) "\x48\xb9", 2); // rdx
						push_ex((char *) &o->rchild->num, 8); // 64 bit #
					}
				}
			} else {
				if (o->op != '/') {
					push_ex((char *) "\x48\xb8", 2); // rax
					push_ex((char *) &o->lchild->num, 8); // 64 bit #
					push_ex((char *) "\x48\xba", 2); // rdx
					push_ex((char *) &o->rchild->num, 8); // 64 bit #
				} else {
					push_ex((char *) "\x48\xb8", 2); // rax
					push_ex((char *) &o->lchild->num, 8); // 64 bit #
					push_ex((char *) "\x48\xb9", 2); // rdx
					push_ex((char *) &o->rchild->num, 8); // 64 bit #
				}
			}
			o->push_instr(this);
		} else {
			// push number to rdx
			push_ex((char *) "\x48\xba", 2);
			push_ex((char *) o->num, 8); // 64 bit #
		}
	} else {
		printf("(nil)");
	}
}

void parse::parse_all() {
	root = gentree();
	printf((char *) "postfix: "); root->print(); printf((char *) "\n");
	setup_stack();
	eval_tree(root);
	reset_stack();
	printf((char *) "result: %lld\n", ((int64_t (*)(int64_t)) mem)(8));
	delete root;
	toki = 0;
	root = NULL;
}
