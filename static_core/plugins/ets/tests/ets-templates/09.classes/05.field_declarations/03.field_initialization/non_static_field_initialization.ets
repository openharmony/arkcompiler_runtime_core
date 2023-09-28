/*---
desc: Non-static field initialization.
assert: If there is an initializer in a non-static field declaration, then the initializer has the semantics of an assigment to the declared variable.
---*/

class A {
  public x: int = 5;
}

function main(): void {
    let instance: A = new A();

    assert instance.x == 5;
}
