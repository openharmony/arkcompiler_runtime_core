/*---
desc: A class inherits fкom its direct superinterface.
assert: A class inherits fom its direct superclass and direct superinterfaces all non-private fields of the superclass and superinterfaces that are both accessible to code in the class and not hidden by a declaration in the class.
---*/

interface A {
  x: int;
}

interface C {
  y: boolean;
}

class B implements A, C{

}

function main(): void {
    let instance: B = new B();

    assert instance.x == 0;
    assert instance.y == false;
}
