{% for ni in integer_mul %}

/*---
desc: Multiplication of integer numbers
params: {{ni.x}} * {{ni.y}} == {{ni.result}}
---*/

function main(): void {
  assert {{ni.x}} * {{ni.y}} == {{ni.result}}; 
}
{% endfor %}