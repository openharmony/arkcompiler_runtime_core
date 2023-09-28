/*---
desc: Declaring local class in method body in static context.
assert: A local class can be declared in a static context and is not considered an inner class of any enclosing class or interface.
tags: [compile-only]
---*/

class A {
  public static bar(): void {
    inner class B {}
  }
}
