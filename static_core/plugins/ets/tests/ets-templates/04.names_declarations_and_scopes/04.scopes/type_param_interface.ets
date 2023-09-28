/*---
desc: >-
  The scope of a type parameter name in a class or interface declaration is
  that entire declaration, excluding static member declarations.
---*/

let T: Number = 1.0

interface I<T extends Number> {
    // abstract method
    foo(p: T): T;

    // default method
    bar(p: T): T {
        return p * 4.0;
    }

    // static method
    static baz(): Number {
        return T * 10.0;
    }
}

class C<T extends Number> implements I<T> {
    fld1: T;
    fld2: T;

    // overridden interface method
    public override foo(p: T): T {
        this.fld1 = p;
        this.fld2 = this.fld1 * 2;
        return this.fld2;
    }
}

function main(): void {
    let c: C<Number> = new C<Number>();

    assert c.foo(2.0) == 4.0
    assert c.bar(2.0) == 8.0
    assert I.baz() == 10.0

    // module level scope is not affected
    assert T == 1.0
}
