{% for v in values %}
/*---
desc: check unary plus operation for unsigned short integer operand
---*/

function main(): void {
  const v: ushort = {{v.value}}
  assert +(v) == {{v.result}}
}

{% endfor %}
