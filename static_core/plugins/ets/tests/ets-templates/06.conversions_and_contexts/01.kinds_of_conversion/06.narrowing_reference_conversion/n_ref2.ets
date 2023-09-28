{% for c in cases %}
/*---
desc: >-
    A narrowing reference conversion exists from reference type S to a reference
    type T if all of the following are true
tags: {{c.tags}}
---*/

{{c.defs}}

function main(): int {
  let src: {{c.stype}} = {{c.sval|safe}};
  let dst: {{c.dtype}} = {{c.dval|safe}};
  if (dst.res != 42) {
    return 1;
  }
  return 0;
}
{% endfor %}
