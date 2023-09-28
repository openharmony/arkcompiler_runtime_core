{% for c in cases %}
/*---
desc: Casting contexts allow the use of an unboxing conversion.
---*/

function main(): int {
  {%- for t in c['types'] %}
  let s{{loop.index}}: {{t.type}} = new {{t.type}}({{t.val}});
  let d{{loop.index}}: {{t.type|lower}} = s{{loop.index}} as {{t.type|lower}};
  if (d{{loop.index}} != {{t.val}}) {
      return 1;
  }
  {%- endfor %}
  return 0;
}
{% endfor %}
