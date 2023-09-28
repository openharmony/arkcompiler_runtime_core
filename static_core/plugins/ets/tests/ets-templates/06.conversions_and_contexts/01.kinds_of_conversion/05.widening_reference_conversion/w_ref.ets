{% for t in cases %}
/*---
desc: >-
    A widening reference conversion exists from any reference type S to any reference
    type T, provided S is a subtype of T.
---*/

{{t.defs}}

function main(): int {
  let src: {{t.stype}} = {{t.src|safe}};
  let dst: {{t.dtype}} = src as {{t.dtype}};
  if (src !== dst ) {  // reference equality
      return 1;
  }
  return 0;
}
{% endfor %}
