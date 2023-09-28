{% for v in values %}
/*---
desc: check greater or equal operation for two floats
---*/

function main(): void {
  const a: float = {{v.left}} as float
  const b: float = {{v.right}} as float
  assert (a >= b) == {{v.result}}
}

{% endfor %}
