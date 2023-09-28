{% for v in values %}
/*---
desc: check bitwise complement of long integers
---*/

function main(): void {
  const v: long = {{v.value}}
  assert ~(v) == {{v.result}}
}

{% endfor %}
