{% for v in values %}
/*---
desc: check less or equal operation for short integer
---*/

function main(): void {
  const a: short = {{v.left}}
  const b: short = {{v.right}}
  assert (a <= b) == {{v.result}}
}

{% endfor %}
