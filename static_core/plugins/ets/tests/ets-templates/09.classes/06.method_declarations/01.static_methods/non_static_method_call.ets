/*---
desc: Non-static method call.
assert: An instance method is always invoked with respect to an object, whith becomes the current object to whitch the keyword this and super refer during execution of the method body.
---*/

class A {
  public foo(): int {
    return 5;
  }
}

class B extends A {
  public x: int;

  public static run(): int {
    return 5;
  }

  public bar(): int {
    this.x = 5;
    return super.foo() + this.x;
  }
}

function main(): void {
  let instance: B = new B();

  assert instance.bar() == 10;
}
