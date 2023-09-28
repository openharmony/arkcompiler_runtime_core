/*---
desc: Class constructors overloading
assert: Overloading of constructors is identical in behavior to overloading of methods. The overloading is resolved at compile time by each class instance creation expression.
---*/

class A {
  public x: int;

  public constructor() {
    this.x = 5;
  }

  public constructor(a: int) {
    this.x = a;
  }

  public constructor(a: int, b: char) {
    this.x = 10;
  }

  public constructor(a: int, b: char, d: Boolean) {
    this.x = 15;
  }
}

function main(): void {
  let i1: A = new A();
  let i2: A = new A(7);
  let i3: A = new A(7, c's');
  let i4: A = new A(7, c's', true);

  assert i1.x == 5;
  assert i2.x == 7;
  assert i3.x == 10;
  assert i4.x == 15;
}
