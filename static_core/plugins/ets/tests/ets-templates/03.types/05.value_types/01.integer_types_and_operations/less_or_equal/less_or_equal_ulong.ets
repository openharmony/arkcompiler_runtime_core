{% for v in values %}
/*---
desc: check less or equal operation for unsigned long integer
---*/

function main(): void {
  const a: ulong = {{v.left}}
  const b: ulong = {{v.right}}
  assert (a <= b) == {{v.result}}
}

{% endfor %}
