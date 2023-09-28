/*---
desc: This reference on object.
assert: The keyword this used as a primary expression denotes a value that is a reference to the object for which the instance method was called or to the object being constructed.
tags: [compile-only]
---*/

class A {
  public x: int;

  public foo(): void { 
    bar(this);
  }
}

function bar(a: A): void {

}
