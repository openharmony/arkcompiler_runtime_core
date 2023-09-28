{% for operand in operands %}

/*---
desc: Type of a numeric conditional expression
---*/

function main(): void {
  let x{{operand.type}} = {{operand.first}} ? {{operand.second}} : {{operand.third}};
  assert {{operand.check}};
}

{% endfor %}