{% for v in values %}
/*---
desc: check less than operation for long integer
---*/

function main(): void {
  const a: long = {{v.left}}
  const b: long = {{v.right}}
  assert (a < b) == {{v.result}}
}

{% endfor %}
