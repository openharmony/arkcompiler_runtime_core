{% for v in values %}
/*---
desc: check prefix increment for char operand
---*/

function main(): void {
  let value: char = {{v.value}}
  let result: char = ++value
  assert value == {{v.result}}
  assert result == {{v.result}}
}

{% endfor %}
