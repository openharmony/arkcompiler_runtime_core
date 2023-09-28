{% for lit in incorrect_float_literals %}
/*---
desc: Incorrect Floating-point literals
tags: [negative, compile-only]
params: {{lit}}
---*/

let x = {{lit}};
function main(): void {}
{% endfor %}
