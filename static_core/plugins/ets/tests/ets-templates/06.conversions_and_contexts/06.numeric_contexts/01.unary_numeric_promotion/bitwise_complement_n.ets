{% for c in cases %}
/*---
desc: >-
    Unary numeric promotion is performed on expressions in the following situation:
    The operand of a bitwise complement operator.
params: {{c.type}} cannot be promoted
tags: [compile-only, negative]
---*/

function foo(a: Object): int {
  return 0;
}

function foo(a: String): int {
  return 0;
}

function foo(a: Integral): int {
  return 0;
}

function foo(a: Floating): int {
  return 0;
}

function foo(a: Double): int {
  return 0;
}

function foo(a: Float): int {
  return 0;
}

function foo(a: boolean): int {
  return 0;
}

function foo(a: int): int {
  return 0;
}

function foo(a: double): int {
  return 0;
}

function foo(a: float): int {
  return 0;
}

function main(): int {
  let v: {{c.type}} = {{c.val}};
  return foo(~v);
}
{% endfor %}
