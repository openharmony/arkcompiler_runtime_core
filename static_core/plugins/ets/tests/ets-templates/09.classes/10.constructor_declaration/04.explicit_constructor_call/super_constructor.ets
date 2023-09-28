/*---
desc: Superclass constructor calls (used to call a direct superclass constructor), called Unqualified superclass constructor calls that start with the keyword super
---*/

class A {
  public x: int;

  public constructor(a: int) {
    this.x = a;
  }
}

class B extends A {
  public constructor() {
    super(5);
  }
}

function main(): void {
  let i1: B = new B();

  assert i1.x == 5;
}
