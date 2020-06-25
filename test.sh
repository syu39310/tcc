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

assert 0 "0;"
assert 42 "42;"
assert 21 "5+20-4;"
assert 41 " 12 + 34 - 5 ;"
assert 47 '5+6*7;'
assert 15 '5*(9-6);'
assert 4 '(3+5)/2;'

assert 0 '0==1;'
assert 1 '42==42;'
assert 1 '0!=1;'
assert 0 '42!=42;'

assert 1 '0<1;'
assert 0 '1<1;'
assert 0 '2<1;'
assert 1 '0<=1;'
assert 1 '1<=1;'
assert 0 '2<=1;'

assert 1 '1>0;'
assert 0 '1>1;'
assert 0 '1>2;'
assert 1 '1>=0;'
assert 1 '1>=1;'
assert 0 '1>=2;'

# Step9:１文字のローカル変数
assert 4 'a = 3;b=1;a + b;'
assert 14 'a = 3;
b = 5 * 6 - 8;
a + b / 2;'

# Step10:複数文字のローカル変数
assert 6 'foo = 1;
bar = 2 + 3;
foo + bar;'

# Step11:return文
assert 14 'a = 3;
b = 5 * 6 - 8;
return a + b / 2;'

# ステップ12: 制御構文を足す
# if true case
assert 2 '
if(1==1)
  return 2;
return 3;'
# if false case
assert 3 '
if(1==2)
  return 2;
return 3;'
# if else true
assert 2 '
if(1==1)
  return 2;
else
  return 3;'
# if else false
assert 3 '
if(1==2)
  return 2;
else
  return 3;'
# while
assert 3 '
while(1)
  return 3;'
assert 3 '
a = 1;
while(a<3)
  a = a+1;
return a;'
# for
assert 3 '
for (;;)
  return 3;
'
assert 0 '
for (a=0;;) 
  return a;
'
assert 5 '
for (a=0;a<5;)
  a = a + 5;
return a;
'
assert 2 '
for (a=2;;a=a+1)
  return a;
'
assert 10 '
for (a=0;a<10;a=a+1)
  1;
return a;
'
# if-while
assert 1 '
a=1;
if (a=1)
  while (a=1)
    return a;
'
# if-for
assert 5 '
a=1;
if (a=1) 
  for (;a < 5;)
    a = a + 1;
return a;
'
# while-if
assert 2 '
a = 1;
while (a==1)
  if (a==1)
    a = a + 1;
return a;
'
# for-if
assert 3 '
for (a=0;a < 5; a = a + 1)
  if (a==3)
    return a;
'
# block
assert 1 '
{return 1;}
'
assert 3 '
a = 1;
if (a = 1) {
  a = 2;
  b = 3;
}
return 3;
'
assert 5 '
for(a=0; a <= 10; a=a+1) {
  if (a==2) 
    b=b+1;
  if (a==4) 
    b=b+1;
  if (a==6) {
    b=b+1;
    b=b+1;
    b=b+1;
  }
}
return b;
'
# func call
assert 0 '
foo();
'

# func call args
assert 1 '
foo1(1);
'
assert 2 '
foo2(1, 2);
'

assert 3 '
foo3(1, 2, 3);
'
assert 4 '
foo4(1, 2, 3, 4);
'
assert 5 '
foo5(1, 2, 3, 4, 5);
'
assert 6 '
foo6(1, 2, 3, 4, 5, 6);
'

echo OK
