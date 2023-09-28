{% for v in values %}
/*---
desc: check less or equal operation for unsigned integer
---*/

function main(): void {
  const a: uint = {{v.left}}
  const b: uint = {{v.right}}
  assert (a <= b) == {{v.result}}
}

{% endfor %}
