{% for v in values %}
/*---
desc: check bitwise complement of integer
---*/

function main(): void {
  const v: int = {{v.value}}
  assert ~(v) == {{v.result}}
}

{% endfor %}
