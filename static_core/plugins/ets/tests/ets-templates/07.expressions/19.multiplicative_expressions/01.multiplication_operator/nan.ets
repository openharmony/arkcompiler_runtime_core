{% for ni in nan %}

/*---
desc: Multiplications with NaN
params: {{ni.x}} * {{ni.y}} == {{ni.result}}
---*/

function main(): void {
  let res = {{ni.x}} * {{ni.y}};
  assert Double.isNaN(res);
}
{% endfor %}
