{% for c in cases %}
/*---
desc: Casting contexts allow the use of the boxing conversion followed by the widening reference conversion.
---*/

function main(): int {
  {%- for t in c['types'] %}
  let p{{loop.index}}: {{t.ptype}} = {{t.expr|safe}};
  let r{{loop.index}}: {{t.rtype}} = p{{loop.index}} as {{t.rtype}};
  if (r{{loop.index}} === null) {
      return 1;
  }
  {%- endfor %}
  return 0;
}
{% endfor %}
