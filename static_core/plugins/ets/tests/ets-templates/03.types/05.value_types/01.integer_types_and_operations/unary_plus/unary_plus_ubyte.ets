{% for v in values %}
/*---
desc: check unary plus operation for unsigned byte operand
---*/

function main(): void {
  const v: ubyte = {{v.value}}
  assert +(v) == {{v.result}}
}

{% endfor %}
