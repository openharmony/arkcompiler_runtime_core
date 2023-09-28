/*---
desc: Static field 'f' have reference in static variable.
assert: For a reference to a static field f declared in class or interface C, it is a compile-time error if. The reference appears either in a static variable's initializer of C or in a static initializer of C.
tags: [compile-only, negative]
---*/

class C {
  public static a: int = f + 5;
  public static f: int;
}
