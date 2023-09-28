{% for v in values %}
/*---
desc: check unary plus operation for short integer operand
---*/

function main(): void {
  const v: short = {{v.value}}
  assert +(v) == {{v.result}}
}

{% endfor %}
