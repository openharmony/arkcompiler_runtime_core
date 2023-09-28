{% for c in cases %}
/*---
desc: In a 'for-of' statement type 'expr' must be either array or string.
tags: [compile-only, negative]
---*/

let res = ""

function main(): int {
    res = ""
    {{c.use|indent}}
    return 1;
}
{% endfor %}
