/*---
desc: Access to a hiden field by access expression with 'super' keyword.
assert: A hidden field can be accessed by using a qualified name if it is static, or by using a field access expression that contains the keyword super or a cast to a superclass type.
---*/

class A {
  public x: int;
}

class B extends A{
  public x: boolean;

  public getX(): int {
    return super.x;
  }
}

function main(): void {
    let instance: B = new B();

    assert instance.getX() == 0;
}
