{% for c in cases %}
{%- for op in ops %}
/*---
desc: >-
    Unary numeric promotion is performed on expressions in the following situation:
    Each operand, separately, of a shift operator.
params: "{{op.name}}, {{c.vtype}} or {{c.rtype}} cannot be promoted to primitive integral."
tags: [compile-only, negative]
---*/

function main(): int {
  let v: {{c.vtype}} = {{c.vval}};
  let r: {{c.rtype}} = {{c.rval}};
  if (v {{op.op}} r != 0) {
    return 1;
  }
  return 0;
}
{% endfor %}
{% endfor %}
