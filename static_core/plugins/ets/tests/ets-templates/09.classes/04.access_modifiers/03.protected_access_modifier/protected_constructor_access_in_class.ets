/*---
desc: Check constructor with protected access modifier in class.
assert: Member or constructor with protected modifier can be accessed only within their declaring class and classes derived from their declaring class.
---*/

final class A {
  x: int;
  y: int;

  protected constructor(x: int, y: int) {
    this.x = x;
    this.y = y;
  }

  public constructor(x: int) {
    this(x, 1);
  }
}

function main(): void {
  let instance: A = new A(2);
}
