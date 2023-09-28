{% for c in cases %}
/*---
desc: Type arguments
params: function
---*/

interface I {}

class X implements I {}

final class Y<T> extends X {}

enum Color { Red, Green, Blue }

function foo <T{{c.bound}}> (t: T, v: T): T {
    let tmp: T = t
    t = v
    v = tmp
    return v;
}

{{c.decl}}

function main(): int {
    {{c.use|indent}}
    return 0;
}
{% endfor %}
