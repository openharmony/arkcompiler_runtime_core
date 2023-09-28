{% for v in values %}
/*---
desc: check equality of two long integers
---*/

function main(): void {
  const a: long = {{v.left}}
  const b: long = {{v.right}}
  assert (a == b) == {{v.result}}
}

{% endfor %}
