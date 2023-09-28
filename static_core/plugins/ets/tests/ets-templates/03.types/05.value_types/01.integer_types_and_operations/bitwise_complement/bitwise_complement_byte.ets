{% for v in values %}
/*---
desc: check bitwise complement of byte
---*/

function main(): void {
  const v: byte = {{v.value}}
  assert ~(v) == {{v.result}}
}

{% endfor %}
