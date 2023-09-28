{% for v in values %}
/*---
desc: check postfix increment for byte operand
---*/

function main(): void {
  let value: byte = {{v.value}}
  let result: byte = value++
  assert value == {{v.result}}
  assert result == {{v.value}}
}

{% endfor %}
