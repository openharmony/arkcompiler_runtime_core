{% for c in cases %}
/*---
desc: |-
    A covariant type argument (or a wildcard with an upper bound) is signified by the following syntax, where T is a typeReference:
    out T
    It restricts available methods so it is only allowed to access methods that do not use T or do use ones with T in out-positions only.
    Consequently, a contravariant type argument (or a wildcard with a lower bound) is signified by the following syntax, where T is a typeReference:
    in T
    It restricts available methods so it is only allowed to access methods that do not use T or do use ones with T in in-positions only.
params: class arguments, use-site variance
---*/

class X {}
class Y extends X {}
class Z extends Y {}

class A<T> {
    private t: T

    public constructor(t: T) {
        this.t = t
    }

    public methIn(t: T): void {
        this.t = t
    }

    public methOut(): T {
        return this.t
    }

    public methInOut(t: T): T {
        this.t = t
        return this.t
    }
}

function main(): int {
    {{c.use|indent}}
    return 0;
}
{% endfor %}
