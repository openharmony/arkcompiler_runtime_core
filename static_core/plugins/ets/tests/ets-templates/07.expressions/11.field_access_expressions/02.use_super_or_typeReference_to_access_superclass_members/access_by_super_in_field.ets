/*---
desc: Access to field by keyword super from field initializer.
assert: The forms using the keyword super are valid only in an instance method, instance initializer, or constructor of a class, or in the initializer of an instance variable of a class. If they appear anywhere else, a compile-time error occurs.
---*/

class A {
  public readonly x: int = 5;

}

class B extends A{
  public y: int = super.x;
}

function main(): void {
    let instance: B = new B();

    assert instance.y == 5;
}
