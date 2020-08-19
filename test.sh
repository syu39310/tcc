#!/bin/bash
assert() {
  expected="$1"
  input="$2"

  ./tcc "$input" > tmp.s
  cc -o tmp tmp.s external.o
  ./tmp
  actual="$?"

  if [ "$actual" = "$expected" ]; then
    echo "$input => $actual"
  else
    echo "$input => $expected expected, but got $actual"
    exit 1
  fi
}
assert 0 "int main(){0;}"
assert 42 "int main(){42;}"
assert 21 "int main(){5+20-4;}"
assert 41 "int main(){ 12 + 34 - 5 ;}"
assert 47 'int main(){5+6*7;}'
assert 15 'int main(){5*(9-6);}'
assert 4 'int main(){(3+5)/2;}'

assert 0 'int main(){0==1;}'
assert 1 'int main(){42==42;}'
assert 1 'int main(){0!=1;}'
assert 0 'int main(){42!=42;}'

assert 1 'int main(){0<1;}'
assert 0 'int main(){1<1;}'
assert 0 'int main(){2<1;}'
assert 1 'int main(){0<=1;}'
assert 1 'int main(){1<=1;}'
assert 0 'int main(){2<=1;}'

assert 1 'int main(){1>0;}'
assert 0 'int main(){1>1;}'
assert 0 'int main(){1>2;}'
assert 1 'int main(){1>=0;}'
assert 1 'int main(){1>=1;}'
assert 0 'int main(){1>=2;}'

# Step9:１文字のローカル変数
assert 3 'int main(){a = 3;}'
assert 4 'int main(){a = 3;b=1;a + b;}'
assert 14 'int main(){a = 3;
b = 5 * 6 - 8;
a + b / 2;}'

# Step10:複数文字のローカル変数
assert 6 'int main(){foo = 1;
bar = 2 + 3;
foo + bar;}'

# Step11:return文
assert 14 'int main(){a = 3;
b = 5 * 6 - 8;
return a + b / 2;}'

# ステップ12: 制御構文を足す
# if true case
assert 2 '
int main() {
if(1==1)
  return 2;
return 3;}'
# if false case
assert 3 '
int main() {
if(1==2)
  return 2;
return 3;}'
# if else true
assert 2 '
int main() {
if(1==1)
  return 2;
else
  return 3;}'
# if else false
assert 3 '
int main() {
if(1==2)
  return 2;
else
  return 3;}'
# while
assert 3 '
int main() {
while(1)
  return 3;}'
assert 3 '
int main() {
a = 1;
while(a<3)
  a = a+1;
return a;}'
# for
assert 3 '
int main() {
for (;;)
  return 3;
}'
assert 0 '
int main() {
for (a=0;;) 
  return a;
}'
assert 5 '
int main() {
for (a=0;a<5;)
  a = a + 5;
return a;
}'
assert 2 '
int main() {
for (a=2;;a=a+1)
  return a;
}'
assert 10 '
int main() {
for (a=0;a<10;a=a+1)
  1;
return a;
}'
# if-while
assert 1 '
int main() {
a=1;
if (a=1)
  while (a=1)
    return a;
}'
# if-for
assert 5 '
int main() {
a=1;
if (a=1) 
  for (;a < 5;)
    a = a + 1;
return a;
}'
# while-if
assert 2 '
int main() {
a = 1;
while (a==1)
  if (a==1)
    a = a + 1;
return a;
}'
# for-if
assert 3 '
int main() {
for (a=0;a < 5; a = a + 1)
  if (a==3)
    return a;
}'
# block
assert 1 '
int main() {
{return 1;}
}'
assert 3 '
int main() {
a = 1;
if (a = 1) {
  a = 2;
  b = 3;
}
return 3;
}'
# ifの連続実行を行うと失敗する。
#assert 5 '
#int main() {
#for(a=0; a <= 10; a=a+1) {
#  if (a==2) 
#    b=b+1;
#  if (a==4) 
#    b=b+1;
#  if (a==6) {
#    b=b+1;
#    b=b+1;
#    b=b+1;
#  }
#}
#return b;
#}'
# func call
assert 0 '
int main() {
foo();
}'

# func call args
assert 1 '
int main() {
foo1(1);
}'
assert 2 '
int main() {
foo2(1, 2);
}'

assert 3 '
int main() {
foo3(1, 2, 3);
}'
assert 4 '
int main() {
foo4(1, 2, 3, 4);
}'
assert 5 '
int main() {
foo5(1, 2, 3, 4, 5);
}'
assert 6 '
int main() {
foo6(1, 2, 3, 4, 5, 6);
}'

assert 3 '
int main() {
  a = func1();
  return  a + 2;
}
int func1() {
  return 1;
}
'

assert 21 '
int main() {
  a = test_func(1,2,3,4,5,6);
  return a;
}
int test_func(a,b,c,d,e,f) {
  return a+b+c+d+e+f;
}
'

echo All OK
make clean
