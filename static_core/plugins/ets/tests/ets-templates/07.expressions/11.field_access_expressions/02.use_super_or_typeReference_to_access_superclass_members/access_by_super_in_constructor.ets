/*---
desc: Access to field by keyword super from constructor.
assert: The forms using the keyword super are valid only in an instance method, instance initializer, or constructor of a class, or in the initializer of an instance variable of a class. If they appear anywhere else, a compile-time error occurs.
---*/

class A {
  public x: int;

  public constructor(x: int) {
    this.x = x;
  }
}

class B extends A {
  public constructor() {
    super(5);
  }
}

function main(): void {
    let instance: B = new B();

    assert instance.x == 5;
}
