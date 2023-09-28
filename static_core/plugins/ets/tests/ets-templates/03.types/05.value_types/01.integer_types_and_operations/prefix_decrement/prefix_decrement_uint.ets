{% for v in values %}
/*---
desc: check prefix decrement for unsigned integer operand
---*/

function main(): void {
  let value: uint = {{v.value}}
  let result: uint = --value
  assert value == {{v.result}}
  assert result == {{v.result}}
}

{% endfor %}
