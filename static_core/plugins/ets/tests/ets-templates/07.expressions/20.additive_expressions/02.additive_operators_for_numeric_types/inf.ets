{% for ni in inf %}

/*---
desc: Additions with INF
params: {{ni.x}} + {{ni.y}} == {{ni.result}}
---*/

function main(): void {
  let x: {{ni.type}} = {{ni.x}};
  let y: {{ni.type}} = {{ni.y}};
  assert (x + y) == {{ni.result}};
}
{% endfor %}
