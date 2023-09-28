{% for c in cases %}
/*---
desc: Casting contexts doesn't allow the following conversion.
tags: [compile-only, negative]
---*/

class A {}

function main(): int {
  let s: A = new A();
  let d: {{c.dtype}} = s as {{c.dtype}};
  return 0;
}
{% endfor %}
