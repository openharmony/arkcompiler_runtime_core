{% for lit in incorrect_int_literals %}

/*---
desc: Incorrect integer literals
tags: [negative, compile-only]
params: '{{lit}}'
---*/

let x = {{lit}};
function main(): void {}

{% endfor %}
