{% for v in values %}
/*---
desc: check bitwise complement of unsigned integer
---*/

function main(): void {
  const v: uint = {{v.value}}
  assert ~(v) == {{v.result}}
}

{% endfor %}
