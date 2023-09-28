{% for v in values %}
/*---
desc: check less than operation for unsigned integer
---*/

function main(): void {
  const a: uint = {{v.left}}
  const b: uint = {{v.right}}
  assert (a < b) == {{v.result}}
}

{% endfor %}
