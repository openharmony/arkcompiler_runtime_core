{% for exp in array_creation %}

/*---
desc: Array creation expression
---*/

class A {}

let x = new {{exp}};
function main(): void {}

{% endfor %}