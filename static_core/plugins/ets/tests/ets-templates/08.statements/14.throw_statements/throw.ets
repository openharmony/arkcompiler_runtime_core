{% for t in cases %}
/*---
desc: Throw statements cause an exception or Error to be thrown.
tags: [compile-only]
---*/

{{t.defs}}
let t1 = 1, t2 = 2

function main(): int {
    {{t.use|indent}}
}
{% endfor %}
