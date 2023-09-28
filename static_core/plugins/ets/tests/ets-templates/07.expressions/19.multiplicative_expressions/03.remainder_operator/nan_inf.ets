{% for ni in nan_inf %}

/*---
desc: Remainder with NaN, INF and zeroes
params: {{ni.x}} % {{ni.y}} == {{ni.result}}
---*/

function main(): void {
  assert {{ni.x}} % {{ni.y}} == {{ni.result}};
}
{% endfor %}
