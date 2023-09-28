/*---
desc: A class inherits from its direct superclass and superinterface.
assert: A class inherits from its direct superclass and direct superinterfaces all non-private fields of the superclass and superinterfaces that are both accessible to code in the class and not hidden by a declaration in the class.
---*/

class A {
  public x: int;
}

interface C {
  public y: boolean;
}

class B extends A implements C {

}

function main(): void {
    let instance: B = new B();

    assert instance.x == 0;
    assert instance.y == false;
}
