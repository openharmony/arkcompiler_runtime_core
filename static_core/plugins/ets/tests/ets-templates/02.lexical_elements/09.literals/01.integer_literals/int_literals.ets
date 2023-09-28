/*---
desc: Correct integer literals with bases 16, 10, 8 and 2
tags: [compile-only]
---*/

{% for lit in int_literals %}
let x{{loop.index}} = {{lit}};
{%- endfor %}

function main(): void {}
