{% for v in values %}
/*---
desc: check unary plus operation for unsigned integer operand
---*/

function main(): void {
  const v: uint = {{v.value}}
  assert +(v) == {{v.result}}
}

{% endfor %}
