{% for case in cases %}
/*---
desc: |-
  The following conversion combines both widening and narrowing primitive conversions:
    - byte to char
---*/

function main(): int {
  {%- for t in case['types'] %}
  let src_{{loop.index}}: {{t.stype}} = {{t.src|safe}};
  if ((src_{{loop.index}} as {{t.dtype}}) != ({{t.dst|safe}}) ) {
      return 1;
  }
  {% endfor %}
  return 0;
}
{% endfor %}
