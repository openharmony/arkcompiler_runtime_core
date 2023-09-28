/*---
desc: Non-static field initialization with keyword 'this'.
assert: It there is an initializer in a non-static field declaration, then the following rules apply to the initializer. The initializer may refer to the current object using the keyword this or the keyword super.
---*/

class A {
  public y: int = 5;
  public x: int = this.y;
}

function main(): void {
    let instance: A = new A();

    assert instance.x == 5;
}