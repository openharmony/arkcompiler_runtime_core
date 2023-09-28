/*---
desc: Readonly field initialization in constructor.
assert: A blank readonly class variable must be initialized at the end of every constructor of the class in which it is declared, or a compile-time error occurs.
---*/

class A {
  public readonly x: int;

  public constructor() {
    this.x = 2;
  }
}

function main(): void {
    let instance: A = new A();

    assert instance.x == 2;
}
