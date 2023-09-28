{% for v in values %}
/*---
desc: check less than operation for short integer
---*/

function main(): void {
  const a: short = {{v.left}}
  const b: short = {{v.right}}
  assert (a < b) == {{v.result}}
}

{% endfor %}
