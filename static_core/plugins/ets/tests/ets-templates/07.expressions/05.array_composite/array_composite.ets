{% for item in array_composite %}

/*---
desc: Array composite
params: {{item.array}}
---*/

let x: {{item.type}}[] = {{item.array}};
function main(): void {}

{% endfor %}
