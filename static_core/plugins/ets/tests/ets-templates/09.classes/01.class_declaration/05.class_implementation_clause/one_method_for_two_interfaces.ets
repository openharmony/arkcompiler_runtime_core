/*---
desc: Declare one method from two interfaces.
assert: It is permitted for a single method declaration in a class to implement methods of more than one superinterface.
tags: [compile-only]
---*/

class A implements C, D {
  public override foo(): void {
    let b: int = 5 + 5;
  }
}

interface C {
  foo(): void;
}

interface D {
  foo(): void;
}
