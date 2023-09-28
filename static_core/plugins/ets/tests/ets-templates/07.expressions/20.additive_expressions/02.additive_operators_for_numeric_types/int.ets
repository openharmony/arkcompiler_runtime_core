{% for ni in ints %}

/*---
desc: Additions with int numbers
params: {{ni.x}} + {{ni.y}} == {{ni.result}}
---*/

function main(): void {
  let x: {{ni.type}} = {{ni.x}};
  let y: {{ni.type}} = {{ni.y}};
  assert (x + y) == {{ni.result}};
}
{% endfor %}
