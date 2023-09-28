{% for v in values %}
/*---
desc: check unary plus operation for long integer operand
---*/

function main(): void {
  const v: long = {{v.value}}
  assert +(v) == {{v.result}}
}

{% endfor %}
