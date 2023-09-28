{% for case in cases %}
/*---
desc: >-
    A value x of primitive type T is first converted to a reference value as if by
    giving it as an argument to an appropriate class instance creation expression.
---*/

function main(): int {
  {%- for t in case['types'] %}
  let src_{{loop.index}}: {{t.type}} = ({{t.val|safe}}) as {{t.type}};
  let dst_{{loop.index}}: string = "" + src_{{loop.index}};
  let exp_{{loop.index}}: string = (new {{t.type|capitalize}}(src_{{loop.index}})).toString();
  if (!dst_{{loop.index}}.equals(exp_{{loop.index}})) {
      return 1;
  }
  {% endfor %}
  return 0;
}
{% endfor %}
