{% for v in values %}
/*---
desc: check not equal operation for two byte operands
---*/

function main(): void {
  const a: byte = {{v.left}}
  const b: byte = {{v.right}}
  assert (a != b) == {{v.result}}
}

{% endfor %}
