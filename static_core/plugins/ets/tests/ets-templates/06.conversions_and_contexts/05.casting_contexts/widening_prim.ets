{% for c in cases %}
/*---
desc: Casting contexts allow the use of the widening primitive conversion.
---*/

function main(): int {
  {%- for t in c['types'] %}
  let v{{loop.index}}: {{t.src_type}} = {{t.src_val|safe}};
  if (v{{loop.index}} as {{t.dst_type}} != {{t.dst_val}} as {{t.dst_type}}) {
      return 1;
  }
  {%- endfor %}
  return 0;
}
{% endfor %}
