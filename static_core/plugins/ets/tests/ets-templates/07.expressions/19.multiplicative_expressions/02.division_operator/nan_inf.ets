{% for ni in nan_inf %}

/*---
desc: Division with NaN and INF
params: {{ni.x}} / {{ni.y}} == {{ni.result}}
---*/

function main(): void {
  assert {{ni.x}} / {{ni.y}} == {{ni.result}};
}
{% endfor %}
