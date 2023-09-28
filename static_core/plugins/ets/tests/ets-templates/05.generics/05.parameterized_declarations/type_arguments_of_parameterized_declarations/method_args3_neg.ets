{% for c in cases %}
/*---
desc: It is a compile-time error if a wildcard is used in a parameterization of a function, method, or constructor.
params: method, use-site variance
tags: [compile-only, negative]
---*/

class X {}

class A {
    private v: X

    public constructor(v: X) {
        this.v = v
    }

    public meth1 <V extends X> (): V {
        return this.v
    }

    public meth2 <V extends X> (v: V): void {
        this.v = v
    }
}

function main(): int {
    let a: A = new A(new X())
    {{c.use|indent}}
    return 0;
}
{% endfor %}
