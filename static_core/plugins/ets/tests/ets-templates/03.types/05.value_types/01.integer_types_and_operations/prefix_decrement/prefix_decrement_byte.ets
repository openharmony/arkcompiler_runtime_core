{% for v in values %}
/*---
desc: check prefix decrement for byte operand
---*/

function main(): void {
  let value: byte = {{v.value}}
  let result: byte = --value
  assert value == {{v.result}}
  assert result == {{v.result}}
}

{% endfor %}
