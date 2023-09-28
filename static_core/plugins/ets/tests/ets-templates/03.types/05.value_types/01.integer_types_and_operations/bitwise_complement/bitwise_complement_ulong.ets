{% for v in values %}
/*---
desc: check bitwise complement of unsigned long integer
---*/

function main(): void {
  const v: ulong = {{v.value}}
  assert ~(v) == {{v.result}}
}

{% endfor %}
