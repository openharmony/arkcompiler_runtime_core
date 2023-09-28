{% for e in paren_expr %}

/*---
desc: Parenthesized expression
---*/

let x = 1;
let y = {{e}};

function main(): void {
  assert x == y;
}

{% endfor %}