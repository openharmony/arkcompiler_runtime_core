{% for c in cases %}
/*---
desc: It is a compile-time error if a wildcard is used in a parameterization of a function, method, or constructor.
params: function, use-site variance
tags: [compile-only, negative]
---*/

class X {}

let tmp: Object | null

function foo1 <T> (t: T) {
    tmp = t
}

function foo2 <T> (t: T): T {
    return t;
}

function main(): int {
    {{c.use|indent}}
    return 0;
}
{% endfor %}
