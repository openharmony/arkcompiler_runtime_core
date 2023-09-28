/*---
desc: Access to constant class variable from inner class.
assert: Local classes have access to instance members of the enclosing class and local variables if such are declared constant (i.e., a variable or parameter whose value remains unchanged after initialization).
tags: [compile-only]
---*/

class A {
  public const x: int = 1;

  inner class B {
    public bar(): void {
      let y: int = x + 1;
    }
  }
}