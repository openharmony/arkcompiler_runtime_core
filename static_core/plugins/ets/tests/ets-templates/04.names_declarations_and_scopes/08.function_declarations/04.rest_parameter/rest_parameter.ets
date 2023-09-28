{% for c in cases %}
/*---
desc: >-
    A rest parameter allows to make a function or method that take an unbounded
    number of arguments. A rest parameter is marked with ... symbol before parameter
    name.
---*/

function {{c.sig}} {
    {{c.def|indent}}
}

function main(): int {
    {{c.call|indent}}
    return 1;
}
{% endfor %}
