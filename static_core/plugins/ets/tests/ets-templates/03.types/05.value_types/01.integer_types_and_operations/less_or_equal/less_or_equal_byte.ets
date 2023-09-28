{% for v in values %}
/*---
desc: check less or equal operation for byte
---*/

function main(): void {
  const a: byte = {{v.left}}
  const b: byte = {{v.right}}
  assert (a <= b) == {{v.result}}
}

{% endfor %}
