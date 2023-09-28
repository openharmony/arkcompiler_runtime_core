/*---
desc: Oerride default method from superinterface
assert: Each default method of a superinterface of the class may optionally be overridden by a method in the class; if not, the default method is typically inherited and its behavior is as specified by its default body.
---*/

class A implements C {
  public override foo(): int {
    return 10;
  }
}

interface C {
  foo(): int {
    return 5;
  };
}

function main(): void {
  let a: A = new A()

  assert a.foo() == 10;
}
