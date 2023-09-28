{% for c in cases %}
/*---
desc: >-
    Unary numeric promotion is performed on expressions in the following situation:
    The index expression in an array access expression.
params: {{c.type}} cannot be promoted to int
tags: [compile-only, negative]
---*/

function main(): int {
  let v: {{c.type}} = {{c.val}};
  let arr: {{c.type}}[] = new {{c.type}}[{{c.len}} + 1];
  arr[v] = v;
  return 0;
}
{% endfor %}
