{% for v in values %}
/*---
desc: check less than operation for char
---*/

function main(): void {
  const a: char = {{v.left}}
  const b: char = {{v.right}}
  assert (a < b) == {{v.result}}
}

{% endfor %}
