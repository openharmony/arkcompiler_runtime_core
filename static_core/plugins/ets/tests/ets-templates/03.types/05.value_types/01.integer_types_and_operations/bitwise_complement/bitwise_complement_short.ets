{% for v in values %}
/*---
desc: check bitwise complement of short integer
---*/

function main(): void {
  const v: short = {{v.value}}
  assert ~(v) == {{v.result}}
}

{% endfor %}
