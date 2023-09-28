{% for c in cases %}
/*---
desc: Casting contexts allow the use of the widening reference conversion.
---*/

function main(): int {
  {%- for t in c['types'] %}
  let s{{loop.index}}: {{t.stype}} = {{t.expr|safe}};
  let d{{loop.index}}: {{t.dtype}} = s{{loop.index}} as {{t.dtype}};
  if (d{{loop.index}} === null) {
      return 1;
  }
  {%- endfor %}
  return 0;
}
{% endfor %}
