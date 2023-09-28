{% for v in values %}
/*---
desc: check unary minus operation for char operand
---*/

function main(): void {
  const value: char = {{v.value}}
  assert -(value) == {{v.result}}
}

{% endfor %}
