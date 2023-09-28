{% for case in cases %}
/*---
desc: >-
    A value of the null type (the null reference is the only such value) may be
    assigned to any reference type, resulting in a null reference of that type.
---*/

function main(): int {
  {%- for t in case['types'] %}
  let v_{{t}}: {{t}}? = null;
  if (v_{{t}} !== null) {
      return 1;
  }

  type ArrayOf{{t}} = {{t}}[];
  let a_{{t}}: ArrayOf{{t}}? = null;
  if (a_{{t}} !== null) {
    return 1;
  }

  type FuncOf{{t}} = ({{t}}): {{t}};
  let f_{{t}}: FuncOf{{t}}? = null;
  if (f_{{t}} !== null) {
    return 1;
  }

  {% endfor %}
  return 0;
}
{% endfor %}