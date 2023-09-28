{% for v in values %}
/*---
desc: check not equal operation for two integer operands
---*/

function main(): void {
  const a: int = {{v.left}}
  const b: int = {{v.right}}
  assert (a != b) == {{v.result}}
}

{% endfor %}
