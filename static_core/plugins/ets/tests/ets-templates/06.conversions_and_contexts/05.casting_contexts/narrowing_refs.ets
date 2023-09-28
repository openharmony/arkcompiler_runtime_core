{% for c in cases %}
/*---
desc: Casting contexts allow the use of the narrowing reference conversion.
---*/

function main(): int {
  {%- for t in c['types'] %}
  let s{{loop.index}}: {{t.stype}} = new {{t.dtype}}({{t.pval}});
  let d{{loop.index}}: {{t.dtype}} = s{{loop.index}} as {{t.dtype}};
  if (d{{loop.index}}.unboxed() != {{t.pval}}) {
      return 1;
  }
  {%- endfor %}
  return 0;
}
{% endfor %}
