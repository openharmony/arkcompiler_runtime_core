/*---
desc: >-
  The scope of a type parameter name in a class or interface declaration is
  that entire declaration, excluding static member declarations.
tags: 
- compile-only
- negative
---*/

let T: Number = 1.0

class C<T extends Number> {
    fld1: T;
    fld2: T;

    public foo(p: T): T {
        this.fld1 = p;
        this.fld2 = this.fld1 * 2; // CTE, cannot apply arithmetic to type parameters
        return this.fld2;
    }

    public static bar(): Number {
        return T * 2;
    }
}

function main(): void {
    let c: C<Number> = new C<Number>();

    assert c.foo(2.0) == 4.0
    assert C.bar() == 2.0

    // module level scope is not affected
    assert T == 1.0
}
