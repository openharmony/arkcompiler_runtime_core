{% for v in values %}
/*---
desc: check unary minus operation for unsigned long integer operand
---*/

function main(): void {
  const value: ulong = {{v.value}}
  assert -(value) == {{v.result}}
}

{% endfor %}
