{% for v in values %}
/*---
desc: check unary minus operation for long integer operand
---*/

function main(): void {
  const value: long = {{v.value}}
  assert -(value) == {{v.result}}
}

{% endfor %}
