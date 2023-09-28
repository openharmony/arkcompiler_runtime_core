{% for ni in inf %}

/*---
desc: Multiplications with INF
params: {{ni.x}} * {{ni.y}} == {{ni.result}}
---*/

function main(): void {
  let res = {{ni.x}} * {{ni.y}};
  assert res == {{ni.result}}
}
{% endfor %}
