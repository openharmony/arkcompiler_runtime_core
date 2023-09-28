{% for v in values %}
/*---
desc: check unary minus operation for unsigned integer operand
---*/

function main(): void {
  const value: uint = {{v.value}}
  assert -(value) == {{v.result}}
}

{% endfor %}
