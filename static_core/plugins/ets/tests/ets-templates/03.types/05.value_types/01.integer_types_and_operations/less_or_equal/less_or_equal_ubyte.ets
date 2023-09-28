{% for v in values %}
/*---
desc: check less or equal operation for unsigned byte
---*/

function main(): void {
  const a: ubyte = {{v.left}}
  const b: ubyte = {{v.right}}
  assert (a <= b) == {{v.result}}
}

{% endfor %}
