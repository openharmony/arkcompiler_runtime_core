{% for c in cases %}
/*---
desc: Type arguments
params: constructor
tags: [compile-only, negative]
---*/

interface I {}

interface J {}

class X implements I {}

class Y {}

class A {
    private t: Object | null

    public constructor <T{{c.bound}}> (t: T) {
        this.t = t
    }
}

enum Color { Red, Green, Blue }

enum Size { S, M, L, XL }

{{c.decl}}

function main(): int {
    {{c.use|indent}}
    return 0;
}
{% endfor %}
