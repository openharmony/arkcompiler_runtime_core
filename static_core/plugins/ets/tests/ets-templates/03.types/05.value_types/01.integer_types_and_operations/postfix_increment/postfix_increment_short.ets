{% for v in values %}
/*---
desc: check postfix increment for short integer operand
---*/

function main(): void {
  let value: short = {{v.value}}
  let result: short = value++
  assert value == {{v.result}}
  assert result == {{v.value}}
}

{% endfor %}
