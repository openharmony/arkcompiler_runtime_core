{% for t in cases %}
/*---
desc: Throw statements cause an exception to be thrown.
tags: [compile-only, negative]
---*/

{{t.defs}}

function main(): int {
    {{t.use|indent}}
    return 0;
}
{% endfor %}
