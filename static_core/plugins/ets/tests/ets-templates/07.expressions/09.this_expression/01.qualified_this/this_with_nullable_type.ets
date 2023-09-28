/*---
desc: This in class method refer too nulable type.
assert: Any lexically enclosing instance (§??and §??) can be referred to by explicitly qualifying the keyword this.
tags: [compile-only]
---*/

class A {
  public x: Int|null;

  public foo(x: int): void { 
    this.x = x;
  }
}