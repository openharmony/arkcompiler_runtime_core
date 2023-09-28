{% for v in values %}
/*---
desc: check postfix increment for unsigned short integer operand
---*/

function main(): void {
  let value: ushort = {{v.value}}
  let result: ushort = value++
  assert value == {{v.result}}
  assert result == {{v.value}}
}

{% endfor %}
