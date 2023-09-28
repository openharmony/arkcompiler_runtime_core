{% for v in values %}
/*---
desc: check greater or equal for two short integers
---*/

function main(): void {
  const a: short = {{v.left}}
  const b: short = {{v.right}}
  assert (a >= b) == {{v.result}}
}

{% endfor %}
