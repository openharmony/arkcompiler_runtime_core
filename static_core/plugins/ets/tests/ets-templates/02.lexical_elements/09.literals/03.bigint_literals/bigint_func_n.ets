{% for f in bigint_functions %}
/*---
desc: >-
    Built-in functions allow to convert strings representing numbers or any integer values into bigint ones.
tags: [compile-only, negative]
---*/

let x: bigint = {{f}};
function main() {}
{% endfor %}
