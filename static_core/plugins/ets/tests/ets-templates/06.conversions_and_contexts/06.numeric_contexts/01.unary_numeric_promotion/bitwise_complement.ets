{% for c in cases %}
/*---
desc: >-
    Unary numeric promotion is performed on expressions in the following situation:
    The operand of a bitwise complement operator.
params: {{c.type}}
---*/

function foo(a: Object): int {
  return 1;
}

function foo(a: byte): int {
  return 2;
}

function foo(a: char): int {
  return 3;
}

function foo(a: short): int {
  return 4;
}

function foo(a: int): int {
  return 5;
}

function foo(a: long): int {
  return 6;
}

function foo(a: float): int {
  return 7;
}

function foo(a: double): int {
  return 8;
}

function main(): int {
  let v: {{c.type}} = {{c.val}};
  let r0 = foo(~v);
  let r1 = foo(~ ~v);
  let r2 = foo(~ ~ ~v);
  let r3 = ~v;
  let r4 = ~ ~ v;
  let r5 = ~ ~ ~ v;
  if (
    r0 == {{c.r[0]}} &&
    r1 == {{c.r[1]}} &&
    r2 == {{c.r[2]}} &&
    r3 == {{c.r[3]}} &&
    r4 == {{c.r[4]}} &&
    r5 == {{c.r[5]}}
  ) {
    return 0;
  }
  return 1;
}
{% endfor %}
