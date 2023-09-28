/*---
desc: Access to a hiden field using cast.
assert: A hidden field can be accessed by using a qualified name if it is static, or by using a field access expression that contains the keyword super or a cast to a superclass type.
---*/

class A {
  public x: int;
}

class B extends A{
  public x: boolean;
}

function main(): void {
    let instance: A = new B();

    assert instance.x == 0;
}
