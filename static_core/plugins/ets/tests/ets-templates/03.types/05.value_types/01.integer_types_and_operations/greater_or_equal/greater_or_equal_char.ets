{% for v in values %}
/*---
desc: check greater or equal for two chars
---*/

function main(): void {
  const a: char = {{v.left}}
  const b: char = {{v.right}}
  assert (a >= b) == {{v.result}}
}

{% endfor %}
