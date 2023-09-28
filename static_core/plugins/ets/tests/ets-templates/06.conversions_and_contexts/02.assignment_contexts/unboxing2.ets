{% for case in cases %}
/*---
desc: Assignment contexts allow the use an unboxing conversion followed by a widening primitive conversion.
params: Conversion from '{{case.src_type}}' to '{{case.dst_type}}'
---*/

function main(): int {
  {% for v in case['values'] %}
  let src_{{loop.index}}: {{case.src_type}} = new {{case.src_type}}({{v|safe}});
  let dst_{{loop.index}}: {{case.dst_type}} = src_{{loop.index}};   // unboxing in this assignment
  if (dst_{{loop.index}} != (src_{{loop.index}}.unboxed() as {{case.dst_type}})) {
      return 1;
  }
  {% endfor %}
  return 0;
}
{% endfor %}
