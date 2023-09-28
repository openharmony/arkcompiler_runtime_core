{% for v in values %}
/*---
desc: check prefix increment for long integer operand
---*/

function main(): void {
  let value: long = {{v.value}}
  let result: long = ++value
  assert value == {{v.result}}
  assert result == {{v.result}}
}

{% endfor %}
