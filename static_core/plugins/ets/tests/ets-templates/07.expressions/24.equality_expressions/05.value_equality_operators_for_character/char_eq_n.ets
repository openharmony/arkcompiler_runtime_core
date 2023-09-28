{% for c in cases %}
/*---
desc: >-
    The operation is a character equality if: both operands of a value equality operator are of type char;
    or one operand is of type char and the other is of type Char.
    The character equality operators are associative.
tags: [compile-only, negative]
---*/

function main(): void {
  let c1 = {{c.c1}}
  let c2 = {{c.c2}}
  assert c1 {{c.op}} c2
}
{% endfor %}
