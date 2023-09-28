{% for c in cases %}
/*---
desc: >-
    A rest parameter allows to make a function or method that take an unbounded
    number of arguments. A rest parameter is marked with ... symbol before parameter
    name.
tags: [compile-only, negative]
---*/

function {{c.sig}} {
    {{c.def|indent}}
}

function main(): int {
    return 0;
}
{% endfor %}
