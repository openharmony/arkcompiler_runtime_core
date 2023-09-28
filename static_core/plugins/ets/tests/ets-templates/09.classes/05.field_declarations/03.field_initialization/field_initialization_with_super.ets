/*---
desc: Non-static field initialization with keyword 'super'.
assert: It there is an initializer in a non-static field declaration, then the following rules apply to the initializer. The initializer may refer to the current object using the keyword this or the keyword super.
---*/

class A {
  public y: int = 5;
}

class B extends A {
  public x: int = super.y
}

function main(): void {
    let instance: B = new B();

    assert instance.x == 5;
}
