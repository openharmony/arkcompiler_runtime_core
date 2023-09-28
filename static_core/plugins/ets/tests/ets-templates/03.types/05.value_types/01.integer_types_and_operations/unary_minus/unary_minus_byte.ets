{% for v in values %}
/*---
desc: check unary minus operation for byte operand
---*/

function main(): void {
  const value: byte = {{v.value}}
  assert -(value) == {{v.result}}
}

{% endfor %}
