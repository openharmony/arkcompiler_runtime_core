{% for c in cases %}
/*---
desc: Type arguments
params: method
tags: [compile-only, negative]
---*/

interface I {}

class X implements I {}

class Y {}

enum Color { Red, Green, Blue }

enum Size { S, M, L, XL }

class A <V> {
    private v: V

    public meth <T{{c.bound}}> (v: V, t: T): T {
        this.v = v
        let tmp: T = t
        return tmp;
    }
}

{{c.decl}}

function main(): int {
    let a: A<X> = new A<X>()
    {{c.use|indent}}
    return 0;
}
{% endfor %}
