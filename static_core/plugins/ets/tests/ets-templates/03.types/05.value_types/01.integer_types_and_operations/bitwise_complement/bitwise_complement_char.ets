{% for v in values %}
/*---
desc: check bitwise complement of char
---*/

function main(): void {
  const v: char = {{v.value}}
  assert ~(v) == {{v.result}}
}

{% endfor %}
