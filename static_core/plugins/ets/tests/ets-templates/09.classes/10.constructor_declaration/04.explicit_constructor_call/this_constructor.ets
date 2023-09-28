/*---
desc: Alternate constructor calls that begin with the keyword this
---*/

class A {
  public x: int;

  public constructor() {
    this(5);
  }

  public constructor(a: int) {
    this.x = a;
  }
}

function main(): void {
  let i1: A = new A();
  let i2: A = new A(7);

  assert i1.x == 5;
  assert i2.x == 7;
}
