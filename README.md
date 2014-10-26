infix
=====

Simple interpreter for infix syntax.

Uses a concurrent lexer/parser to create a binary tree from infix statements and then evaluate them recursively.

```sh
$ clang++ -g main.cc -std=c++11
$ ./a.out
8 * 8
prefix: * 8 8
postfix: 8 8 *
infix: (8 * 8)
=> 64
```
