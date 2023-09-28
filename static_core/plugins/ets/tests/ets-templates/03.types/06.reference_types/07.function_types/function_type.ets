{% for variable in funtion_variables %}

/*---
desc: Function types 
params: '{{variable}}'
tags: [compile-only]
---*/

{{variable}}

function main(): void {}

{% endfor %}
