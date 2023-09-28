{% for c in cases %}
/*---
desc: Type arguments
params: method
---*/

interface I {}

class X implements I {}

final class Y<T> extends X {}

enum Color { Red, Green, Blue }

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
