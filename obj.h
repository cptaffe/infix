
// obj

#ifndef INFIX_OBJ_H_
#define INFIX_OBJ_H_


#include <stdlib.h>
#include <stdint.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
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

static int getprec(char c) {
	if (c == '+' || c == '-') {return 0;}
	else if (c == '*' || c == '/') {return 1;}
	else if (c == '(') {return 2;}
	else {return -1;}
}

Obj::Obj(int type) {
	num = 0;
	rchild = NULL; lchild = NULL;
	this->type = (Objtype) type;
}

// new obj from string
Obj::Obj(char *tok) {
	rchild = NULL; lchild = NULL;
	if (isdigit(*tok)) {
		type = TYPE_NUM;
		num = atoi(tok); // num
	} else {
		type = TYPE_OP;
		op = tok[0];
		prec = getprec(op);
	}
}

// recursive free
Obj::~Obj() {
	if (rchild != NULL) {delete rchild;}
	if (lchild != NULL) {delete lchild;}
}

string *Obj::prefix() {
	string *str = new string();
	prefix(str);
	return str;
}

void Obj::prefix(string *str) {
	if (this != NULL) {
		if (type == TYPE_OP) {
			str->append(toStr());
			str->push_back(' ');
			lchild->prefix(str);
			str->push_back(' ');
			rchild->prefix(str);
			str->push_back(' ');
		} else {str->append(toStr());}
	} else {str->append(toStr());}
}

string *Obj::postfix() {
	string *str = new string();
	postfix(str);
	return str;
}

void Obj::postfix(string *str) {
	if (this != NULL) {
		if (type == TYPE_OP) {
			lchild->postfix(str);
			str->push_back(' ');
			rchild->postfix(str);
			str->push_back(' ');
			str->append(toStr());
			str->push_back(' ');
		} else {str->append(toStr());}
	} else {str->append(toStr());}
}

string *Obj::infix() {
	string *str = new string();
	infix(str);
	return str;
}

void Obj::infix(string *str) {
	if (this != NULL) {
		if (type == TYPE_OP) {
			str->push_back('(');
			lchild->infix(str);
			str->push_back(' ');
			str->append(toStr());
			str->push_back(' ');
			rchild->infix(str);
			str->push_back(')');
		} else {str->append(toStr());}
	} else {str->append(toStr());}
}

bool Obj::isValid() {
	if (this == NULL) {return false;}
	else if (type == TYPE_OP) {
		if (lchild->isValid() && rchild->isValid()) {return true;}
		else {return false;}
	} else {
		return true;
	}
}

int64_t Obj::eval() {
	if (this == NULL) {return 0;}
	if (type == TYPE_OP) {
		if (op == '+') {return lchild->eval() + rchild->eval();}
		else if (op == '-') {return lchild->eval() - rchild->eval();}
		else if (op == '*') {return lchild->eval() * rchild->eval();}
		else if (op == '/') {return lchild->eval() / rchild->eval();}
		else{return 0;}
	} else {
		return num;
	}
}

string Obj::toStr() {
	if (this != NULL) {
		if (type == TYPE_OP) {
			return string(1, op);
		} else {
			return to_string(num);
		}
	} else {
		return "nil";
	}
}

#endif // INFIX_OBJ_H
