{% for c in cases %}
/*---
desc: >-
    Unary numeric promotion is performed on expressions in the following situation:
    Each dimension expression in an array creation expression.
params: {{c.type}} cannot be promoted to int
tags: [compile-only, negative]
---*/

function main(): int {
  let v: {{c.type}} = {{c.val}};
  let arr: {{c.type}}[][] = new {{c.type}}[v][v];
  if (arr.length != {{c.len}}) {
    return 1;
  }
  return 0;
}
{% endfor %}
