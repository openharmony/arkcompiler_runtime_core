{% for c in cases %}
/*---
desc: Casting contexts allow the use of an unboxing conversion followed by a widening primitive conversion.
---*/

function main(): int {
  {%- for t in c['types'] %}
  let s{{loop.index}}: {{t.type}} = new {{t.type}}({{t.val}});
  let d{{loop.index}}: {{t.dtype}} = s{{loop.index}} as {{t.dtype}};
  if (d{{loop.index}} != {{t.dval}}) {
      return 1;
  }
  {%- endfor %}
  return 0;
}
{% endfor %}
