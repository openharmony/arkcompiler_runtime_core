{% for v in values %}
/*---
desc: check not equal operation for two long integer operands
---*/

function main(): void {
  const a: long = {{v.left}}
  const b: long = {{v.right}}
  assert (a != b) == {{v.result}}
}

{% endfor %}
