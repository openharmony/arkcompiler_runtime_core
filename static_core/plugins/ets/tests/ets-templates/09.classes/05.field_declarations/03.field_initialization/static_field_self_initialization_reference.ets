/*---
desc: Non-static field 'f' have reference in static.
assert: For a reference to a static field f declared in class or interface C, it is a compile-time error if. The reference appears in the initializer of f's own declaration or at a point prior to f's declaration.
tags: [compile-only, negative]
---*/

class C {
  public static f: int = f + 5;
}
