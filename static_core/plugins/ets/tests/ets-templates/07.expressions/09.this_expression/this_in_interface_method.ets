/*---
desc: This in interface method.
assert: The keyword this can only be used as an expression in the body of an instance method of a class, enum or interface.
tags: [compile-only]
---*/

interface A {
  bar(): int {
    return 5;
  }

  public foo() { 
    let x = this.bar();
  }
}