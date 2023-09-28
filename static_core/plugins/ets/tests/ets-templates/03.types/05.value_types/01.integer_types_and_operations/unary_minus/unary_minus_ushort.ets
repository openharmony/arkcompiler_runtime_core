{% for v in values %}
/*---
desc: check unary minus operation for unsigned short integer operand
---*/

function main(): void {
  const value: ushort = {{v.value}}
  assert -(value) == {{v.result}}
}

{% endfor %}
