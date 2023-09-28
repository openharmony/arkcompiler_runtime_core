{% for c in cases %}
/*---
desc: Type arguments
params: interface arguments, invariant
tags: [compile-only, negative]
---*/

interface I {}
class X implements I {}
class Y {}

interface J<T{{c.bound}}> {}
class A<T{{c.bound}}> implements J<T> {
    private t: T

    public constructor(t: T) {
        this.t = t
    }
}

enum Color { Red, Green, Blue }
enum Size { S, M, L, XL, XXL }
{{c.decl}}
function main(): int {
    {{c.use|indent}}
    return 0;
}
{% endfor %}
