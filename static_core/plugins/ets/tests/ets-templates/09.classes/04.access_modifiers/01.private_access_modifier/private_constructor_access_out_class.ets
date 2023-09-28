/*---
desc: Check constructor with private access modifier in class.
assert: Member or constructor with private modifier can be accessed only within their declaring class.
tags: [negative, compile-only]
---*/

class A {
  x: int;
  y: int;

  private constructor(x: int, y: int) {
    this.x = x;
    this.y = y;
  }

  public constructor(x: int) {
    this(x, 1);
  }
}

function main(): void {
  let instance: A = new A(2, 3);
}
