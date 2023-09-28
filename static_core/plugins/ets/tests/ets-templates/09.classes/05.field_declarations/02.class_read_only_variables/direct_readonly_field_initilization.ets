/*---
desc: Direct readonly field initialization.
assert: A blank readonly class variable must be initialized by a static initializer of the class in which it is declared, or a compile-time error occurs.
---*/

class A {
  public readonly x: int = 1;
}

function main(): void {
    let instance: A = new A();

    assert instance.x == 1;
}
