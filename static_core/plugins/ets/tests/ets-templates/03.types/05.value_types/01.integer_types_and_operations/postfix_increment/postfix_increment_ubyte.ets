{% for v in values %}
/*---
desc: check postfix increment for unsigned byte operand
---*/

function main(): void {
  let value: ubyte = {{v.value}}
  let result: ubyte = value++
  assert value == {{v.result}}
  assert result == {{v.value}}
}

{% endfor %}
