/*---
desc: Check constructor with protected access modifier in derived class.
assert: Member or constructor with protected modifier can be accessed only within their declaring class and classes derived from their declaring class.
---*/

class A {
  x: int;
  y: int;

  protected constructor(x: int, y: int) {
    this.x = x;
    this.y = y;
  }
}

final class B extends A {
  public constructor(x: int) {
    super(x, 1);
  }
}

function main(): void {
  let instance: B = new B(2);
}
