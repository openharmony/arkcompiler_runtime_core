{% for v in values %}
/*---
desc: check prefix increment for integer operand
---*/

function main(): void {
  let value: int = {{v.value}}
  let result: int = ++value
  assert value == {{v.result}}
  assert result == {{v.result}}
}

{% endfor %}
