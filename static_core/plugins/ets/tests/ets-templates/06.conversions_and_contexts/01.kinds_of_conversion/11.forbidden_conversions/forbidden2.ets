{% for c in cases %}
/*---
desc: >-
    Any conversion that is not explicitly allowed is forbidden.
tags: [negative]
---*/

{{c.defs}}

function main(): int {
  let src: {{c.stype}} = {{c.sval|safe}};
  let dst: {{c.dtype}} = src as {{c.dtype}};
  return 0;   // shouldn't reach this line, ClassCastException is expected
}
{% endfor %}
