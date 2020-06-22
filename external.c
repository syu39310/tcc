#include <stdio.h>

int foo() { printf("OK\n"); }
int foo1(int a) { printf("OK[%d]\n", a); }
int foo2(int a, int b) { printf("OK[%d, %d]\n", a, b); }
int foo3(int a, int b, int c) { printf("OK[%d, %d, %d]\n", a, b, c); }
int foo4(int a, int b, int c, int d) { printf("OK[%d, %d, %d, %d]\n", a, b, c, d); }
int foo5(int a, int b, int c, int d, int e) { printf("OK[%d, %d, %d, %d, %d]\n", a, b, c, d, e); }
int foo6(int a, int b, int c, int d, int e, int f) { printf("OK[%d, %d, %d, %d, %d, %d]\n", a, b, c, d, e, f); }