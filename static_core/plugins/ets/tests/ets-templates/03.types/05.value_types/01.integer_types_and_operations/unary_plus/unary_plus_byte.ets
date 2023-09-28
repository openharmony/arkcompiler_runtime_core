{% for v in values %}
/*---
desc: check unary plus operation for byte operand
---*/

function main(): void {
  const v: byte = {{v.value}}
  assert +(v) == {{v.result}}
}

{% endfor %}
