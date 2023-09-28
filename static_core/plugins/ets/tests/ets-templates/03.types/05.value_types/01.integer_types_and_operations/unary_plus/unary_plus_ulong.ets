{% for v in values %}
/*---
desc: check unary plus operation for unsigned long integer operand
---*/

function main(): void {
  const v: ulong = {{v.value}}
  assert +(v) == {{v.result}}
}

{% endfor %}
