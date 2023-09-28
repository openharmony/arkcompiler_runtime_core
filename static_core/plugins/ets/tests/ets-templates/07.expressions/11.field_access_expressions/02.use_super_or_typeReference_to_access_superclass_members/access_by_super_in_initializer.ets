/*---
desc: Access to field by keyword super from initialize.
assert: The forms using the keyword super are valid only in an instance method, instance initializer, or constructor of a class, or in the initializer of an instance variable of a class. If they appear anywhere else, a compile-time error occurs.
tags: [negative]
---*/

class A {
  public x: int;

  public constructor(x: int) {
    this.x = super(5);
  }
}

class B extends A{
  public static y: int;

  static { 
    y = super.x;
  }
}

function main(): void {
    let instance: C<B> = new C<B>();

    assert instance.bar() == 1;
}
