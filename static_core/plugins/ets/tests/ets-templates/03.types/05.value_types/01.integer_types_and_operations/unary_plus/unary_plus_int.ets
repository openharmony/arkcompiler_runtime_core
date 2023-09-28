{% for v in values %}
/*---
desc: check unary plus operation for integer operand
---*/

function main(): void {
  const v: int = {{v.value}}
  assert +(v) == {{v.result}}
}

{% endfor %}
