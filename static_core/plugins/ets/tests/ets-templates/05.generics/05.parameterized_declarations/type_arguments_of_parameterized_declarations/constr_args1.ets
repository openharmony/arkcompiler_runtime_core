{% for c in cases %}
/*---
desc: Type arguments
params: constructor
---*/

interface I {}

class X implements I {}

class Y extends X {}

class A {
    private t: Object | null

    public constructor <T{{c.bound}}> (t: T) {
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
