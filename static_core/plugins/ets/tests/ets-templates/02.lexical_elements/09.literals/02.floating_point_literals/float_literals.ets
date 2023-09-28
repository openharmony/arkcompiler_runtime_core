/*---
desc: Floating-point literals
tags: [compile-only]
---*/

{% for f in float_literals %}
let x{{loop.index}} = {{f}};
{%- endfor %}

function main(): void {}
