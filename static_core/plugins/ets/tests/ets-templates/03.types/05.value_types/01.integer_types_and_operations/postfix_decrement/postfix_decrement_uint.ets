{% for v in values %}
/*---
desc: check postfix decrement for unsigned int operand
---*/

function main(): void {
  let value: uint = {{v.value}}
  let result: uint = value--
  assert value == {{v.result}}
  assert result == {{v.value}}
}

{% endfor %}
