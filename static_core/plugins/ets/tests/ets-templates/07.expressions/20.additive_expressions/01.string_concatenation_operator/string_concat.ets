{% for sc in string_concat %}

/*---
desc: String concatenation
params: |
  {{sc.x}} + {{sc.y}} == {{sc.result}}
---*/

function main(): void {
  assert ({{sc.x}} + {{sc.y}}) == {{sc.result}};
}
{% endfor %}