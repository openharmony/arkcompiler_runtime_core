/*---
desc: Declaring local class in method body with refer on constant variable.
assert: A local class can only be referred to with a simple name (not a qualified or canonical name), i.e., a local class cannot be considered if a canonical name is required.
tags: [compile-only, negative]
---*/

class A {
  public static const x: Object;

  public static bar(): void {
    inner class B {
      public bar(): void {
        let y: Object = x;
      }
    }
  }
}
