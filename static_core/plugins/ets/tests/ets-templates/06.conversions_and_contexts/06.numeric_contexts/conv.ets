{% for c in cases %}
/*---
desc: Numeric contexts apply to the operands of an arithmetic operator.
params: {{c.type1}}, {{c.type2}}
tags: {{c.tags}}
---*/

{{c.defs}}

function main(): int {
  let v1: {{c.type1}} = {{c.v1|safe}};
  let v2: {{c.type2}} = {{c.v2|safe}}
  if (v1 {{c.op}} v2 != {{c.res}}) {
    return 1;
  }
  return 0;
}
{% endfor %}
