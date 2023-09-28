{% for c in cases %}
/*---
desc: Type arguments
params: class arguments, invariant
tags: [compile-only, negative]
---*/

interface I {}

class X implements I {}

class Y {}

enum Color { Red, Green, Blue }

enum Size { S, M, L, XL, XXL }

class A<T{{c.bound}}> {
    private t: T;

    public constructor(t: T) {
        this.t = t;
    }
}

{{c.decl}}

function main(): int {
    {{c.use|indent}}
    return 0;
}
{% endfor %}
