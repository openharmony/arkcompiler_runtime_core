/*---
desc: Access to a static hiden field.
assert: A hidden field can be accessed by using a qualified name if it is static, or by using a field access expression that contains the keyword super or a cast to a superclass type.
---*/

class A {
  public static x: int;
}

class B extends A{
  public x: boolean;

  public getX(): int {
    return A.x;
  }
}

function main(): void {
    let instance: B = new B();

    assert instance.getX() == 0;
}
