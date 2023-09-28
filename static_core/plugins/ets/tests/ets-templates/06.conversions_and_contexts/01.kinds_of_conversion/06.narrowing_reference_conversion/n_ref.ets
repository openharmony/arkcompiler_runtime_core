{% for c in cases %}
/*---
desc: >-
    A narrowing reference conversion exists from reference type S to a reference
    type T if all of the following are true
---*/

{{c.defs}}

function main(): int {
  let src: {{c.stype}} = {{c.val|safe}};
  let dst: {{c.dtype}} = src as {{c.dtype}};
  if (src !== dst ) {  // reference equality
      return 1;
  }
  return 0;
}
{% endfor %}
