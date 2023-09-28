/*---
desc: Check constructor with internal access modifier.
assert: Member or constructor with private modifier can be accessed only eithin their declaring class.
---*/

class A {
  x: int;
  y: int;

  internal constructor(x: int, y: int) {
    this.x = x;
    this.y = y;
  }

  internal constructor(x: int) {
    this(x, 1);
  }
}

function main(): void {
  let instance: A = new A(2, 3);
  let instanc: A = new A(2);
}
