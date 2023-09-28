{% for v in values %}
/*---
desc: check greater operation for two unsigned bytes
---*/

function main(): void {
  const a: ubyte = {{v.left}}
  const b: ubyte = {{v.right}}
  assert (a > b) == {{v.result}}
}

{% endfor %}
