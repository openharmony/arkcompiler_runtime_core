{% for v in values %}
/*---
desc: check greater or equal for two unsigned shorts
---*/

function main(): void {
  const a: ushort = {{v.left}}
  const b: ushort = {{v.right}}
  assert (a >= b) == {{v.result}}
}

{% endfor %}
