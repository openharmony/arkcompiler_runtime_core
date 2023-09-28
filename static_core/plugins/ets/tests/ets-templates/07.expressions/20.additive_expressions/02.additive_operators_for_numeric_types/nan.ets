{% for ni in nan %}

/*---
desc: Additions with NaN and INF
params: {{ni.x}} + {{ni.y}} == {{ni.result}}
---*/

function main(): void {
  let x: {{ni.type}} = {{ni.x}};
  let y: {{ni.type}} = {{ni.y}};
  assert Double.isNaN(x + y);
}
{% endfor %}
