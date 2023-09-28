/*---
desc: Several paths by which the same field declaration is inherited from an interface.
assert: There might be several paths by which the same field declaration is inherited from an interface. In such asituation, the field is considered to be inherited only once, and it may be referred to by its simple name without ambiguity.
---*/

interface A {
  x: int;
}

class C implements A {

}

class B extends C implements A {
}

function main(): void {
    let instance: B = new B();

    assert instance.x == 0;
}
