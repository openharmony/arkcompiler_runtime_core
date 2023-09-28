/*---
desc: call constructor through keyword this
assert: Explicit constructor call statement is the only position in which the keyword this can be used in a direct call expression this().
---*/

class A {
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