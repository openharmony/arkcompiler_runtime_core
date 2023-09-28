{% for c in cases %}
/*---
desc: Assignment contexts allow the use of a boxing conversion
params: Conversion from '{{c.src_type}}' to '{{c.dst_type}}'
---*/

function main(): int {
  {% for v in c['values'] %}
  let src_{{loop.index}}: {{c.src_type}} = {{v|safe}};
  let dst_{{loop.index}}: {{c.dst_type}} = src_{{loop.index}};   // boxing in this assignment
  if (dst_{{loop.index}}.unboxed() != src_{{loop.index}}) {
      return 1;
  }
  {% endfor %}
  return 0;
}
{% endfor %}
