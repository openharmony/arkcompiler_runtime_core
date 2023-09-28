{% for c in cases %}
/*---
desc: Type arguments
params: class arguments, invariant
---*/

interface I {}

class X implements I {}

enum Color { Red, Green, Blue }

class A<T{{c.bound}}> {
    private t: T

    public constructor(t: T) {
        this.t = t
    }
}

{{c.decl}}
function main(): int {
    {{c.use|indent}}
    return 0;
}
{% endfor %}
