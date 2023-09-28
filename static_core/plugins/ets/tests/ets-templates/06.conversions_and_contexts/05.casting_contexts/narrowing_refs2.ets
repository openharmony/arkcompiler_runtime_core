{% for c in cases %}
/*---
desc: Casting contexts allow the use of the narrowing reference conversion followed by an unboxing conversion.
---*/

function main(): int {
  {%- for t in c['types'] %}
  let s{{loop.index}}: {{t.stype}} = new {{t.dtype}}({{t.pval}});
  let d{{loop.index}}: {{t.dtype|lower}} = s{{loop.index}} as {{t.dtype|lower}};
  if (d{{loop.index}} != {{t.pval}}) {
      return 1;
  }
  {%- endfor %}
  return 0;
}
{% endfor %}
