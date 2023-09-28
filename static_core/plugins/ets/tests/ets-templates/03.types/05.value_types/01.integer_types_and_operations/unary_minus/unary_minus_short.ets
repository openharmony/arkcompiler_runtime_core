{% for v in values %}
/*---
desc: check unary minus operation for short integer operand
---*/

function main(): void {
  const value: short = {{v.value}}
  assert -(value) == {{v.result}}
}

{% endfor %}
