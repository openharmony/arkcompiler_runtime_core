{% for c in cases %}
/*---
desc: Casting contexts allow the use of the narrowing primitive conversion.
---*/

function main(): int {
  {%- for t in c['types'] %}
  let x{{loop.index}}: {{t.src_type}} = {{t.src_val|safe}};
  let y{{loop.index}}: {{t.dst_type}} = {{t.dst_val|safe}};
  if (x{{loop.index}} as {{t.dst_type}} != y{{loop.index}}) { // narrowing primitive casting here
      return 1;
  }
  {%- endfor %}
  return 0;
}
{% endfor %}
