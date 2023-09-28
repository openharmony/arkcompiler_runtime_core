{% for vd in var_decl %}

/*---
desc: A variable declaration introduces a named variable and optionally assigns it an initial value
tags: [compile-only, negative]
---*/

{{vd}}

function main(): void {}
{% endfor %}
