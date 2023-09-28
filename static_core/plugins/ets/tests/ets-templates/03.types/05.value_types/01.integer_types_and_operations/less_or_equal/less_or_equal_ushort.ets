{% for v in values %}
/*---
desc: check less or equal operation for unsigned short integer
---*/

function main(): void {
  const a: ushort = {{v.left}}
  const b: ushort = {{v.right}}
  assert (a <= b) == {{v.result}}
}

{% endfor %}
