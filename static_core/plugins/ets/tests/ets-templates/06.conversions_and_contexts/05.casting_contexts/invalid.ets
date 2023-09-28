{% for c in cases %}
/*---
desc: Casting contexts doesn't allow the following conversion.
tags: [compile-only, negative]
---*/

function main(): int {
  let s: {{c.stype}} = {{c.sval|safe}};
  let d: {{c.dtype}} = s as {{c.dtype}};
  return 0;
}
{% endfor %}
