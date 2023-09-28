{% for v in values %}
/*---
desc: check postfix decrement for unsigned long integer operand
---*/

function main(): void {
  let value: ulong = {{v.value}}
  let result: ulong = value--
  assert value == {{v.result}}
  assert result == {{v.value}}
}

{% endfor %}
