{% for v in values %}
/*---
desc: check greater operation for two unsigned short integers
---*/

function main(): void {
  const a: ushort = {{v.left}}
  const b: ushort = {{v.right}}
  assert (a > b) == {{v.result}}
}

{% endfor %}
