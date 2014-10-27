
// obj

#ifndef INFIX_OBJ_H_
#define INFIX_OBJ_H_

#include <string>

using namespace std;

typedef enum {
	TYPE_OP,
	TYPE_NUM,
} Objtype;

class Obj {
	friend class parse;
	Objtype type;
	union {
		int64_t num;
		struct {
			char op;
			int prec;
		};
	};
	Obj *rchild;
	Obj *lchild;
public:
	Obj(int);
	Obj(char *);
	~Obj();
	bool isValid();
	string toStr();
	string *prefix();
	string *postfix(); // string as postfix
	string *infix(); // string as infix
	void prefix(string *);
	void postfix(string *); // string as postfix
	void infix(string *); // string as infix
	char *instr(); // returns x86 instruction
	int64_t eval(); // evaluate tree
};

#endif // INFIX_OBJ_H
