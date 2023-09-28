{% for case in cases %}
/*---
desc: Assignment contexts allow the use of the identity conversion.
---*/

function main(): int {
  {%- for t in case['types'] %}
  let src_{{loop.index}}: {{t.type}} = {{t.expr|safe}};
  let dst_{{loop.index}}: {{t.type}} = src_{{loop.index}}
  if (src_{{loop.index}} {{t.op}} dst_{{loop.index}}) {
      return 1;
  }
  {% endfor %}
  return 0;
}
{% endfor %}