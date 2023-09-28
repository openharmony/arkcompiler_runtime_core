{% for v in values %}
/*---
desc: check less than operation for unsigned short integer
---*/

function main(): void {
  const a: ubyte = {{v.left}}
  const b: ubyte = {{v.right}}
  assert (a < b) == {{v.result}}
}

{% endfor %}
