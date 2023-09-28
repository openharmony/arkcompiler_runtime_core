{% for v in values %}
/*---
desc: check unary plus operation for char operand
---*/

function main(): void {
  const v: char = {{v.value}}
  assert +(v) == {{v.result}}
}

{% endfor %}
