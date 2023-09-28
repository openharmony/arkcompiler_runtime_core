{% for v in values %}
/*---
desc: check postfix increment for unsigned integer operand
---*/

function main(): void {
  let value: uint = {{v.value}}
  let result: uint = value++
  assert value == {{v.result}}
  assert result == {{v.value}}
}

{% endfor %}
