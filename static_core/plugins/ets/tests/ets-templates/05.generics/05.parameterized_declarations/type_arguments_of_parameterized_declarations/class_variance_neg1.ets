{% for c in cases %}
/*---
desc: >-
    It is a compile-time error if a covariant wildcard is specified for a contravariant type parameter.
    It is a compile-time error if a contravariant wildcard is specified for a covariant type parameter.
params: class arguments
tags: [compile-only, negative]
---*/

class A<{{c.variance}}T> {
    private t: T

    public constructor(t: T) {
        this.t = t
    }
}

function main(): int {
    {{c.use|indent}}
    return 0;
}
{% endfor %}
