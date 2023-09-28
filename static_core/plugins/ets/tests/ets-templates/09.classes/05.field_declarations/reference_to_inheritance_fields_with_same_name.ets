/*---
desc: Refer to inherits field with the same name from its direct superclass and superinterfaces.
assert: However, any attempt within the body of the class to refer to any such field by its simple name will result in a compile-time error, because the reference is ambiguous.
tags: [negative, compile-only]
---*/

class A {
  x: int = 5;
}

interface C {
  x: int = 1;
}

interface D {
  x: int = 1;
}

class B extends A implements C, D{
  public increment(): void {
    this.x++;
  }
}
