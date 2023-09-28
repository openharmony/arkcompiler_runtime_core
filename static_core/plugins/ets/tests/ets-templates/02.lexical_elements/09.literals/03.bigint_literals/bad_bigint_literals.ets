{% for lit in bad_bigint_literals %}
/*---
desc: Incorrect bigint literals
tags: [negative, compile-only]
params: '{{lit}}'
---*/

let x: bigint = {{lit}};
function main() {}
{% endfor %}
