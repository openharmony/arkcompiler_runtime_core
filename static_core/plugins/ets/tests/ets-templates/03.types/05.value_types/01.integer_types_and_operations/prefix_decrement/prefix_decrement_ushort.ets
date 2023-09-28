{% for v in values %}
/*---
desc: check prefix decrement for unsigned short integer operand
---*/

function main(): void {
  let value: ushort = {{v.value}}
  let result: ushort = --value
  assert value == {{v.result}}
  assert result == {{v.result}}
}

{% endfor %}
