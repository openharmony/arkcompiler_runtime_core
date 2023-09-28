{% for c in cases %}
/*---
desc: Casting contexts allow the use of the boxing conversion.
---*/

function main(): int {
  {%- for t in c['types'] %}
  let p{{loop.index}}: {{t.ptype}} = {{t.expr|safe}};
  let r{{loop.index}}: {{t.rtype}} = p{{loop.index}} as {{t.rtype}};
  if (p{{loop.index}} != r{{loop.index}}.unboxed()) {
      return 1;
  }
  {%- endfor %}
  return 0;
}
{% endfor %}
