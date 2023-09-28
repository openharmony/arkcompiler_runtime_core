{% for v in values %}
/*---
desc: check not equal operation for two char operands
---*/

function main(): void {
  const a: char = {{v.left}}
  const b: char = {{v.right}}
  assert (a != b) == {{v.result}}
}

{% endfor %}
