{% for v in values %}
/*---
desc: check postfix decrement for char operand
---*/

function main(): void {
  let value: char = {{v.value}}
  let result: char = value--
  assert value == {{v.result}}
  assert result == {{v.value}}
}

{% endfor %}
