{% for c in cases %}
/*---
desc: Type arguments
params: method
---*/

interface I {}

class X implements I {}

final class Y<T> extends X {}

enum Color { Red, Green, Blue }

class A {
    public meth<T{{c.bound}}>(t: T, v: T): T {
        let tmp: T = t
        t = v
        v = tmp
        return v;
    }
}

{{c.decl}}

function main(): int {
    let a = new A()
    {{c.use|indent}}
    return 0;
}
{% endfor %}
