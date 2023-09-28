{% for v in values %}
/*---
desc: check greater or equal for two unsigned integers
---*/

function main(): void {
  const a: uint = {{v.left}}
  const b: uint = {{v.right}}
  assert (a >= b) == {{v.result}}
}

{% endfor %}
