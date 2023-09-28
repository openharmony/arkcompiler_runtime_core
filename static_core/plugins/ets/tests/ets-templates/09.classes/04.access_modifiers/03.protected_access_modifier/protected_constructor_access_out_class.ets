/*---
desc: Check constructor with protected access modifier out class.
assert: Member or constructor with protected modifier can be accessed only within their declaring class and classes derived from their declaring class.
tags: [negative, compile-only]
---*/

class A {
  x: int;
  y: int;

  protected constructor(x: int, y: int) {
    this.x = x;
    this.y = y;
  }
}

function main(): void {
  let instance: A = new A(2, 3);
}
