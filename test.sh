#!/bin/bash
assert() {
  expected="$1"
  input="$2"

  ./tcc "$input" > tmp.s
  cc -o tmp tmp.s
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

echo OK
