{% for v in values %}
/*---
desc: check unary minus operation for integer operand
---*/

function main(): void {
  const value: int = {{v.value}}
  assert -(value) == {{v.result}}
}

{% endfor %}
