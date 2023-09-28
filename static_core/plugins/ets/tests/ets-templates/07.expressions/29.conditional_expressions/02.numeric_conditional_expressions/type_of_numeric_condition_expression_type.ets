{% for operand in operands %}

/*---
desc: Type of a numeric conditional expression
tags: [compile-only, negative]
---*/

function main(): void {
  let x = {{operand.first}} ? {{operand.second}} : {{operand.third}};
  assert {{operand.check}};
}

{% endfor %}
