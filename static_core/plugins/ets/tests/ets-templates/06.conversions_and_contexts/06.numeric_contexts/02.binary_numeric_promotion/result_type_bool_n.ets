{% for c in cases %}
/*---
desc: |-
    When an operator applies binary numeric promotion to a pair of operands,
    each of which must denote a value that is convertible to a numeric type,
    the following rules apply, in order:
    1. If any operand is of a reference type, it is subjected to unboxing conversion (link to conv).
    2. Widening primitive conversion (link to conv) is applied to convert either or both operands as specified by the following rules:
      •	If either operand is of type double, the other is converted to double.
      •	Otherwise, if either operand is of type float, the other is converted to float.
      •	Otherwise, if either operand is of type long, the other is converted to long.
      •	Otherwise, both operands are converted to type int.
params: {{c.type}}, cannot promote
tags: [compile-only, negative]
---*/

function main(): int {
  let v: {{c.type}} = {{c.val}};
  let r: {{c.rt}} = {{c.rv}};
  let result: int = v {{c.op}} r ? 0 : 1;
  return result;
}
{% endfor %}
