{% for c in cases %}
/*---
desc: It is a compile-time error if a wildcard is used in a parameterization of a function, method, or constructor.
params: constructor, use-site variance
tags: [compile-only, negative]
---*/

class A {
    private t: Object | null

    private constructor() {}

    public constructor <T> (t: T) {
        this()
        this.t = t
    }
}

function main(): int {
    {{c.use|indent}}
    return 0;
}
{% endfor %}
