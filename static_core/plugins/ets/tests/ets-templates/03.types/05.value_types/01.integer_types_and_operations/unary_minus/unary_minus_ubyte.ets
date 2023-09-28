{% for v in values %}
/*---
desc: check unary minus operation for unsigned byte operand
---*/

function main(): void {
  const value: ubyte = {{v.value}}
  assert -(value) == {{v.result}}
}

{% endfor %}
