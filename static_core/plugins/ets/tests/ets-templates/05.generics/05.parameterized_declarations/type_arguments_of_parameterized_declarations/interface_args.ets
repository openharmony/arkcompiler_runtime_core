{% for c in cases %}
/*---
desc: Type arguments
params: interface arguments, invariant
---*/

interface I {}
class X implements I {}

interface J<T{{c.bound}}> {}
class A<T{{c.bound}}> implements J<T> {
    private t: T

    public constructor(t: T) {
        this.t = t
    }
}

enum Color { Red, Green, Blue }
{{c.decl}}
function main(): int {
    {{c.use|indent}}
    return 0;
}
{% endfor %}
