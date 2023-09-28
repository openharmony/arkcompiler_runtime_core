/*---
desc: This reference on object.
assert: The value denoted by this in a lambda body and in the surrounding context is the same.
tags: [compile-only]
---*/

type func = (y: int): int;

class A {
  public x: int;

  public foo() { 
    let bar: func = (a: A): int => { return this.x; };
    return bar(this);
  }
}

function main(): void {
  let instance: A = new A();

  assert instance.foo() == 1;
}