#include <stdio.h>

int foo() { printf("OK\n"); return 0;}
int foo1(int a) { printf("OK[%d]\n", a); return 1;}
int foo2(int a, int b) { printf("OK[%d, %d]\n", a, b); return 2;}
int foo3(int a, int b, int c) { printf("OK[%d, %d, %d]\n", a, b, c); return 3;}
int foo4(int a, int b, int c, int d) { printf("OK[%d, %d, %d, %d]\n", a, b, c, d); return 4;}
int foo5(int a, int b, int c, int d, int e) { printf("OK[%d, %d, %d, %d, %d]\n", a, b, c, d, e); return 5;}
int foo6(int a, int b, int c, int d, int e, int f) { printf("OK[%d, %d, %d, %d, %d, %d]\n", a, b, c, d, e, f); return 6;}