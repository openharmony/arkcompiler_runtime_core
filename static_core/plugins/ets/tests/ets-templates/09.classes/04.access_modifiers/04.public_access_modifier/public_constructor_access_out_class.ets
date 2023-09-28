/*---
desc: Check constructor with public access modifier out class.
assert: Member or constructor with public modifier can be accessed everywhere.
---*/

public class A {
  x: int;
  y: int;

  public constructor(x: int, y: int) {
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
