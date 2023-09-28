{% for v in values %}
/*---
desc: check bitwise complement of unsigned byte
---*/

function main(): void {
  const v: ubyte = {{v.value}}
  assert ~(v) == {{v.result}}
}

{% endfor %}
